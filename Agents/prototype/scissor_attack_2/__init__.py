"""提出フォーマットの実体 (同調減速 ＆ 条件分岐動的射程モデル)"""

import json
import os
import numpy as np
from math import atan2, cos, sin, radians, degrees
from ASRCAISim1.policy import StandalonePolicy  # type: ignore


def getUserAgentClass(args={}):
    from .agent import SimpleAgent
    return SimpleAgent


def getUserAgentModelConfig(args={}):
    with open(os.path.join(os.path.dirname(__file__), "config.json"), "r") as f:
        return json.load(f)


def isUserAgentSingleAsset(args={}):
    return True


def getUserPolicy(args={}):
    return PincerMovementPolicy(args)


class PincerMovementPolicy(StandalonePolicy):
    """【陽動機動同期 ＆ 条件別動的射程 Policy】"""

    def __init__(self, args={}):
        super().__init__()
        args = args if args is not None else {}

        self.flankAngle = float(args.get("flankAngle", 40.0))
        self.flankSteps = int(args.get("flankSteps", 60))

        self.centerHateRangeKm = float(args.get("centerHateRangeKm", 70.0))
        self.vipRushFireRangeKm = float(args.get("vipRushFireRangeKm", 50.0))

        self.disengageSteps = int(args.get("disengageSteps", 20))
        self.marginKm = float(args.get("marginKm", 15.0))

        # 状態管理
        self.agent_states = {}
        self.flank_arrived = False

    def step(self, observation, reward, done, info,
             agentFullName, observation_space, action_space):

        own = np.asarray(observation["own"], dtype=np.float64)
        enemy = np.asarray(observation["enemy"], dtype=np.float64)
        threat = np.asarray(observation["threat"], dtype=np.float64)
        vip = np.asarray(observation.get("vip", np.zeros(7)), dtype=np.float64)

        numTurn = int(action_space["turn"].n)
        numSpeed = int(action_space["speed"].n)

        fired_log = int(own[8])
        role_id = int(own[9])
        elapsed_time = own[11]
        allies_alive = own[12]
        enemies_alive = own[13]
        flank_x = own[14]           # 迂回部隊の平均X座標[km]
        is_enemy_heading_me = (enemy[6] > 0.5) if enemy[0] > 0.5 else False

        # 初期化
        if agentFullName not in self.agent_states:
            self.agent_states[agentFullName] = {
                "phase": "FLANK" if role_id in [1, 4] else "CENTER_ATTACK",
                "count": 0,
                "has_disengaged": False,
                "has_fired": False,
                "prev_vip_y_diff": None
            }

        state = self.agent_states[agentFullName]
        state["count"] += 1
        phase = state["phase"]

        if fired_log == 1:
            state["has_fired"] = True

        # Vc (0〜1100) 比率
        Vc = enemy[5] if enemy[0] > 0.5 else 0.0
        ratio = max(0.0, min(1100.0, Vc)) / 1100.0

        # 750秒超過かつ2機以上優勢時の10km短縮オフセット
        extra_offset = 10.0 if (elapsed_time > 750.0 and allies_alive - enemies_alive >= 2.0) else 0.0

        # -----------------------------------------------------------------
        # 動的射程の決定 (役割・状態に応じた範囲設定)
        # -----------------------------------------------------------------
        if role_id in [1, 4]:  # 迂回部隊
            if state["has_fired"] or is_enemy_heading_me:
                # 1発射撃後、または敵が自機を向いている場合: 45km 〜 60km
                min_r, max_r = 45.0, 60.0
            else:
                # デフォルト引きつけ時: 25km 〜 45km
                min_r, max_r = 25.0, 45.0
            flank_r = max(10.0, (min_r + (max_r - min_r) * ratio) - extra_offset)
            center_normal_r = 55.0

        else:  # 陽動部隊 (Role ID 2, 3)
            if state["has_disengaged"]:
                # 最初の切り返し以降: 45km 〜 60km
                min_r, max_r = 45.0, 60.0
                center_normal_r = max(10.0, (min_r + (max_r - min_r) * ratio) - extra_offset)
            else:
                # 切り返し前: 55km (着任前陽動は70km)
                center_normal_r = max(10.0, 55.0 - extra_offset)
            flank_r = 45.0

        center_hate_r = self.centerHateRangeKm
        vip_r = self.vipRushFireRangeKm

        # -----------------------------------------------------------------
        # 1. 状態遷移 ＆ 条件判定
        # -----------------------------------------------------------------

        # --- [両翼部隊: VIP_RUSH 判定] ---
        if role_id in [1, 4] and phase in ["FLANK", "PINCER"]:
            no_regular_enemy_in_front_20deg = enemy[7] < 0.5
            vip_detected = (vip[0] > 0.5) and (int(vip[5]) == 1)
            same_quadrant = (own[1] * vip[4] > 0) if vip_detected else False

            y_diff_decreasing = False
            if vip_detected:
                curr_y_diff = abs(own[1] - vip[4])
                if state["prev_vip_y_diff"] is not None:
                    if curr_y_diff < state["prev_vip_y_diff"]:
                        y_diff_decreasing = True
                state["prev_vip_y_diff"] = curr_y_diff

            if no_regular_enemy_in_front_20deg and vip_detected and same_quadrant and y_diff_decreasing:
                phase = "VIP_RUSH"
                state["phase"] = phase
                state["count"] = 0

        # --- [両翼部隊: 50km 割込緊急交戦 ＆ 展開完了判定] ---
        if phase == "FLANK":
            if enemy[0] > 0.5 and enemy[2] <= 50.0:
                phase = "PINCER"
                state["phase"] = phase
                state["count"] = 0
                self.flank_arrived = True
            elif state["count"] >= self.flankSteps:
                phase = "PINCER"
                state["phase"] = phase
                state["count"] = 0
                self.flank_arrived = True

        if phase == "PINCER":
            self.flank_arrived = True

        # --- [離脱判定 (DISENGAGE)] ---
        if fired_log == 1 or threat[0] > 0.5:
            if phase != "DISENGAGE":
                state["prev_phase"] = phase
                phase = "DISENGAGE"
                state["phase"] = phase
                state["count"] = 0
                state["has_disengaged"] = True

        elif phase == "DISENGAGE":
            if state["count"] >= self.disengageSteps:
                phase = state.get("prev_phase", "CENTER_ATTACK")
                state["phase"] = phase
                state["count"] = 0

        # -----------------------------------------------------------------
        # 2. 役割別・フェーズ別機動 ＆ 速度・射撃指示
        # -----------------------------------------------------------------
        desiredAz = 0.0
        speedIndex = numSpeed - 1  # 基本は全速 (560 m/s)
        targetIndex = 0

        if phase == "CENTER_ATTACK":
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                fire_range = center_normal_r if self.flank_arrived else center_hate_r
                if enemy[2] <= fire_range:
                    targetIndex = 1
            else:
                desiredAz = 0.0

            # 開幕〜初切り返し前: 迂回部隊(flank_x)とX座標を合わせるよう減速制御
            if not state["has_disengaged"]:
                x_diff = own[0] - flank_x  # 陽動部隊が前に出ている量 [km]
                if x_diff > 2.0:
                    # 前に出すぎている場合は段階的に減速 (400〜480 m/s 付近へ落とす)
                    slow_step = int(min(numSpeed - 1, max(0, (numSpeed - 1) - int(x_diff // 2.0))))
                    speedIndex = slow_step

        elif phase == "FLANK":
            desiredAz = self.flankAngle if role_id == 1 else -self.flankAngle
            speedIndex = numSpeed - 1  # 迂回部隊は全速 (560 m/s)

        elif phase == "PINCER":
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                if enemy[2] <= flank_r:
                    targetIndex = 1
            else:
                desiredAz = 0.0

        elif phase == "VIP_RUSH":
            speedIndex = numSpeed - 1
            if vip[0] > 0.5:
                desiredAz = vip[1]
                if vip[2] <= vip_r:
                    targetIndex = int(vip[6])
            else:
                desiredAz = 0.0

        elif phase == "DISENGAGE":
            speedIndex = numSpeed - 1
            if threat[0] > 0.5:
                desiredAz = threat[1] + 180.0
            elif enemy[0] > 0.5:
                desiredAz = enemy[1] + 180.0
            else:
                desiredAz = 180.0

        # -----------------------------------------------------------------
        # 3. 場外回避 (最優先)
        # -----------------------------------------------------------------
        marginSide, marginFront, marginBack = own[5], own[6], own[7]
        if marginFront < self.marginKm:
            desiredAz = 90.0 if own[1] < 0.0 else -90.0
        elif marginBack < self.marginKm:
            desiredAz = 0.0
        if marginSide < self.marginKm:
            desiredAz = -60.0 if own[1] > 0.0 else 60.0

        turnIndex = self._nearestTurnIndex(desiredAz, numTurn)

        return {"turn": turnIndex, "speed": speedIndex, "target": targetIndex}

    def _nearestTurnIndex(self, desiredAz, numTurn):
        az = (desiredAz + 180.0) % 360.0 - 180.0
        step = 360.0 / numTurn
        index = int(round((az + 180.0) / step)) % numTurn
        return index
