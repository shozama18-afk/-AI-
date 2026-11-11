"""提出フォーマットの実体 (S方式: 1機に1エージェント)"""

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
    return AdvancedTacticsPolicy(args)


# ======================================================================
# Policy : TROLLモード ＋ 相手玉切れ時超近接 ＋ 動的アスペクト ＋ VIP-RUSH
# ======================================================================

class AdvancedTacticsPolicy(StandalonePolicy):
    """【新戦術統合 Policy】

    ■ 新機構:
    1. 通常射程を10km削減:
       - ヘッドオン最大射程: 80km -> 70km
       - 側面・追撃射程: 40km -> 30km
       - VIP射程: 50km -> 40km
    2. 自機玉切れ (own[10] == 0) 時の TROLL モード:
       - 攻撃/VIP-RUSHを停止。通常射程(約70km)よりやや遠い 75km まで接近しては反転退避を反復。
       - 敵に長距離弾を無駄撃ちさせて敵の玉切れを誘発する。
    3. 相手玉切れ (推定敵残弾 == 0) 時の 20km 射程削減:
       - 相手が弾切れの際、反撃の恐れがないため射程をさらに 20km 短縮（超近距離まで引きつける）。
       - 例: 側面・追撃時 10km、VIP 20km まで肉薄して絶対命中を狙う。
    """

    def __init__(self, args={}):
        super().__init__()
        args = args if args is not None else {}
        # 通常射程 (従来より10km短縮)
        self.baseHeadonRangeKm = float(args.get("baseHeadonRangeKm", 70.0))
        self.baseFlankRangeKm = float(args.get("baseFlankRangeKm", 30.0))
        self.baseVipRangeKm = float(args.get("baseVipRangeKm", 40.0))

        self.marginKm = float(args.get("marginKm", 20.0))
        self.disengageSteps = int(args.get("disengageSteps", 20))

        # TROLLモードでの間合い判定 (通常射程70kmより少し遠い75km)
        self.trollTriggerRangeKm = 75.0

        # 全機で共有する敵推定残弾数 (初期値: 4機×4発 = 16発)
        self.estimated_enemy_missiles = 16
        self.known_threat_ids = set()

        # 各機体の状態管理
        self.agent_states = {}

    def step(self, observation, reward, done, info,
             agentFullName, observation_space, action_space):

        own = np.asarray(observation["own"], dtype=np.float64)
        enemy = np.asarray(observation["enemy"], dtype=np.float64)
        threat = np.asarray(observation["threat"], dtype=np.float64)

        numTurn = int(action_space["turn"].n)
        numSpeed = int(action_space["speed"].n)

        marginFront = own[6]
        fired_log = own[8]
        role_id = int(own[9])
        my_ammo = int(own[10])  # 自機の残りミサイル数

        # 機体状態初期化
        if agentFullName not in self.agent_states:
            self.agent_states[agentFullName] = {
                "phase": "WAIT" if role_id in [2, 3] else "ATTACK",
                "count": 0,
                "troll_sub_state": "APPROACH",  # TROLL時: APPROACH or RETREAT
                "prev_enemy_pos": None
            }

        state = self.agent_states[agentFullName]
        state["count"] += 1
        phase = state["phase"]

        # -----------------------------------------------------------------
        # 0. 敵ミサイル残数の追跡 (MWS検知等による更新)
        # -----------------------------------------------------------------
        if threat[0] > 0.5:
            # 簡略化されたカウント管理 (MWS検知時に減算)
            threat_key = f"{round(threat[1], 1)}"
            if threat_key not in self.known_threat_ids:
                self.known_threat_ids.add(threat_key)
                self.estimated_enemy_missiles = max(0, self.estimated_enemy_missiles - 1)

        enemy_is_out_of_ammo = (self.estimated_enemy_missiles <= 0)

        # -----------------------------------------------------------------
        # 1. 自機弾切れ時の TROLL モード起動 ＆ VIP-RUSH 判定
        # -----------------------------------------------------------------
        if my_ammo == 0:
            # ★ 機構①: 自機の玉切れ時は攻撃/VIP-RUSHを完全解除し TROLL モードへ ★
            if phase != "TROLL":
                phase = "TROLL"
                state["phase"] = phase
                state["count"] = 0
                state["troll_sub_state"] = "APPROACH"
        elif marginFront < 50.0 and phase != "VIP_RUSH":
            # VIP-RUSH 復旧 (弾が残っている場合)
            phase = "VIP_RUSH"
            state["phase"] = phase
            state["count"] = 0

        # -----------------------------------------------------------------
        # 2. アスペクト角による動的射程の計算
        # -----------------------------------------------------------------
        # 通常基準値 (10km短縮済み)
        headon_range = self.baseHeadonRangeKm
        flank_range = self.baseFlankRangeKm
        vip_range = self.baseVipRangeKm

        # ★ 機構②: 相手残数(ミサイル)が0の時は、射程をさらに 20km 削減 ★
        if enemy_is_out_of_ammo:
            headon_range = max(15.0, headon_range - 20.0)
            flank_range = max(10.0, flank_range - 20.0)
            vip_range = max(15.0, vip_range - 20.0)

        dynamic_fire_range = headon_range

        if enemy[0] > 0.5:
            own_x, own_y = own[0], own[1]
            enemy_az_rad = radians(enemy[1])
            enemy_dist = enemy[2]

            curr_enemy_pos = np.array([
                own_x + enemy_dist * cos(enemy_az_rad),
                own_y + enemy_dist * sin(enemy_az_rad)
            ])

            prev_pos = state["prev_enemy_pos"]
            if prev_pos is not None:
                d_vec = curr_enemy_pos - prev_pos
                move_dist = np.linalg.norm(d_vec)

                if move_dist > 0.1:
                    enemy_heading_deg = degrees(atan2(d_vec[1], d_vec[0]))
                    los_deg = enemy[1]
                    diff_deg = (enemy_heading_deg - (los_deg + 180.0) + 180.0) % 360.0 - 180.0
                    headon_factor = (cos(radians(diff_deg)) + 1.0) / 2.0
                    dynamic_fire_range = flank_range + (headon_range - flank_range) * headon_factor

            state["prev_enemy_pos"] = curr_enemy_pos

        # -----------------------------------------------------------------
        # 3. 状態遷移 (フェーズ更新)
        # -----------------------------------------------------------------
        if phase == "WAIT":
            if state["count"] >= 30:
                phase = "ATTACK"
                state["phase"] = phase
                state["count"] = 0

        elif phase == "ATTACK":
            if fired_log == 1 or threat[0] > 0.5:
                phase = "DISENGAGE"
                state["phase"] = phase
                state["count"] = 0

        elif phase == "DISENGAGE":
            if state["count"] >= self.disengageSteps:
                phase = "ATTACK"
                state["phase"] = phase
                state["count"] = 0

        # -----------------------------------------------------------------
        # 4. フェーズごとの制御値決定
        # -----------------------------------------------------------------
        desiredAz = 0.0
        speedIndex = numSpeed // 2
        targetIndex = 0

        if phase == "WAIT":
            desiredAz = 0.0
            speedIndex = 0

        elif phase == "ATTACK":
            speedIndex = numSpeed - 1
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                if enemy[2] <= dynamic_fire_range:
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

        elif phase == "VIP_RUSH":
            speedIndex = numSpeed - 1
            if enemy[0] > 0.5:
                desiredAz = enemy[1]
                if enemy[2] <= vip_range:  # 通常40km / 相手玉切れ時20km
                    targetIndex = 1
            else:
                desiredAz = 0.0

        elif phase == "TROLL":
            # ★ 機構①: TROLL モード (弾切れ誘発機動) ★
            # 通常射程(70km)よりやや遠い 75km まで全速接近し、達したら反転退避
            speedIndex = numSpeed - 1
            
            if threat[0] > 0.5:
                # 被弾警報時は即逃走
                desiredAz = threat[1] + 180.0
            elif enemy[0] > 0.5:
                enemy_dist = enemy[2]
                
                # サブステート切り替え
                if state["troll_sub_state"] == "APPROACH" and enemy_dist <= self.trollTriggerRangeKm:
                    state["troll_sub_state"] = "RETREAT"
                    state["count"] = 0
                elif state["troll_sub_state"] == "RETREAT" and (enemy_dist > 85.0 or state["count"] > 15):
                    state["troll_sub_state"] = "APPROACH"
                    state["count"] = 0

                if state["troll_sub_state"] == "APPROACH":
                    desiredAz = enemy[1]  # 挑発接近
                else:
                    desiredAz = enemy[1] + 180.0  # 反転退避
            else:
                desiredAz = 0.0

        # -----------------------------------------------------------------
        # 5. 場外回避 (最優先補正)
        # -----------------------------------------------------------------
        marginSide, marginFront, marginBack = own[5], own[6], own[7]
        if marginFront < self.marginKm and phase != "VIP_RUSH":
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