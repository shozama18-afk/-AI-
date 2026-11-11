"""提出フォーマットの実体 (動的射程短縮 ＆ 750秒優勢時条件適応モデル)"""

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
    """【特定射程短縮 ＆ 750秒/2機優勢時追加短縮 Policy】"""

    def __init__(self, args={}):
        super().__init__()
        args = args if args is not None else {}

        # --- パラメータ設定 ---
        self.flankAngle = float(args.get("flankAngle", 40.0))
        self.flankSteps = int(args.get("flankSteps", 60))
        
        # 1. 対象外の射程（元の設定値を維持）
        self.centerHateRangeKm = float(args.get("centerHateRangeKm", 70.0))      # 中央陽動（変更なし）
        self.vipRushFireRangeKm = float(args.get("vipRushFireRangeKm", 40.0))    # VIP強襲（変更なし）

        # 2. 対象となる射程（通常時から5km短縮）
        self.centerNormalRangeKm = float(args.get("centerNormalRangeKm", 60.0))  # 中央本格攻撃: 60 -> 55km (-5km)
        self.flankFireRangeKm = float(args.get("flankFireRangeKm", 45.0))        # 迂回部隊攻撃: 50 -> 45km (-5km)

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
        vip = np.asarray(observation.get("vip", np.zeros(6)), dtype=np.float64)

        numTurn = int(action_space["turn"].n)
        numSpeed = int(action_space["speed"].n)

        fired_log = int(own[8])
        role_id = int(own[9])
        elapsed_time = own[11]      # 経過時間[秒]
        allies_alive = own[12]      # 味方生存機数
        enemies_alive = own[13]     # 敵機数

        # --- 動的射程計算 ---
        # 750秒超過かつ2機以上優勢な場合、指定の2箇所のみさらに10km短縮
        extra_offset = 0.0
        if elapsed_time > 750.0 and (allies_alive - enemies_alive >= 2.0):
            extra_offset = 10.0

        # 対象のみ10km追加短縮を適用（下限10km）
        center_normal_r = max(10.0, self.centerNormalRangeKm - extra_offset)
        flank_r = max(10.0, self.flankFireRangeKm - extra_offset)

        # 対象外の射程（常に固定）
        center_hate_r = self.centerHateRangeKm
        vip_r = self.vipRushFireRangeKm

        # 初期化
        if agentFullName not in self.agent_states:
            self.agent_states[agentFullName] = {
                "phase": "FLANK" if role_id in [1, 4] else "CENTER_ATTACK",
                "count": 0,
                "prev_vip_y_diff": None
            }

        state = self.agent_states[agentFullName]
        state["count"] += 1
        phase = state["phase"]

        # -----------------------------------------------------------------
        # 1. 状態遷移 ＆ 条件判定
        # -----------------------------------------------------------------
        
        # --- [両翼部隊: VIP_RUSH 判定] ---
        if role_id in [1, 4] and phase in ["FLANK", "PINCER"]:
            no_enemy_in_front_40deg = (enemy[0] < 0.5) or (abs(enemy[1]) > 40.0)
            vip_detected = (vip[0] > 0.5) and (int(vip[5]) == 1)
            same_quadrant = (own[1] * vip[4] > 0) if vip_detected else False

            y_diff_decreasing = False
            if vip_detected:
                curr_y_diff = abs(own[1] - vip[4])
                if state["prev_vip_y_diff"] is not None:
                    if curr_y_diff < state["prev_vip_y_diff"]:
                        y_diff_decreasing = True
                state["prev_vip_y_diff"] = curr_y_diff

            if no_enemy_in_front_40deg and vip_detected and same_quadrant and y_diff_decreasing:
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

        elif phase == "DISENGAGE":
            if state["count"] >= self.disengageSteps:
                phase = state.get("prev_phase", "CENTER_ATTACK")
                state["phase"] = phase
                state["count"] = 0

        # -----------------------------------------------------------------
        # 2. 役割別・フェーズ別機動 ＆ 射撃指示
        # -----------------------------------------------------------------
        desiredAz = 0.0
        speedIndex = numSpeed - 1
        targetIndex = 0

        if phase == "CENTER_ATTACK":
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                # 挟撃部隊着任前は 70km で陽動、着任後は 55km (条件達成時 45km) で本格攻撃
                fire_range = center_normal_r if self.flank_arrived else center_hate_r
                if enemy[2] <= fire_range:
                    targetIndex = 1
            else:
                desiredAz = 0.0

        elif phase == "FLANK":
            desiredAz = self.flankAngle if role_id == 1 else -self.flankAngle
            speedIndex = numSpeed - 1

        elif phase == "PINCER":
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                # 迂回部隊攻撃開始時: 45km (条件達成時 35km)
                if enemy[2] <= flank_r:
                    targetIndex = 1
            else:
                desiredAz = 0.0

        elif phase == "VIP_RUSH":
            speedIndex = numSpeed - 1
            if vip[0] > 0.5:
                desiredAz = vip[1]
                if vip[2] <= vip_r:  # 50km 固定
                    targetIndex = 1
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