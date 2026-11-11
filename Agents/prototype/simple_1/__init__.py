"""提出フォーマットの実体 (S方式: 1機に1エージェント)"""

import json
import os
import numpy as np
from math import atan2, cos, sin, sqrt, degrees, radians, pi
from ASRCAISim1.policy import StandalonePolicy  # type: ignore


# ======================================================================
# 提出に必須の4関数
# ======================================================================

def getUserAgentClass(args={}):
    """使用する Agent クラスを返す。"""
    from .agent import SimpleAgent
    return SimpleAgent


def getUserAgentModelConfig(args={}):
    """Agent に渡す設定を返す。"""
    with open(os.path.join(os.path.dirname(__file__), "config.json"), "r") as f:
        return json.load(f)


def isUserAgentSingleAsset(args={}):
    """1機に1エージェントを割り当てる S方式 なので True を返す。"""
    return True


def getUserPolicy(args={}):
    """行動を決める Policy を返す。"""
    return SimpleRuleBasedPolicy(args)


# ======================================================================
# Policy : Observation を見て action を決める
# ======================================================================

class SimpleRuleBasedPolicy(StandalonePolicy):
    def __init__(self, args={}):
        super().__init__()
        args = args if args is not None else {}
        self.fireRangeKm = float(args.get("fireRangeKm", 80.0))
        self.marginKm = float(args.get("marginKm", 25.0))
        
        # 回避関連ステート
        self.evasion_phase = "NONE"   # "NONE", "STAGE1_DIAGONAL", "STAGE2_BEAM"
        self.evasion_sign = 1.0      # 1.0:右回避, -1.0:左回避
        
        # 追尾・アプローチ関連ステート
        self.target_locked = False
        self.lock_start_dist = 0.0
        self.lock_start_own_pos = (0.0, 0.0)
        self.approach_side = 1.0
        self.prev_enemy_pos = None
        
        # 離脱関連ステート（Stage 3）
        self.last_action_fired = False
        self.disengage_phase = False
        self.disengage_target_dist = 30.0
        self.disengage_step_count = 0  # 離脱経過ステップ数
        self.fire_own_pos = (0.0, 0.0)  # ★射撃時点の自機座標[km]
        
        self.turnTable = np.array(args.get("turnTable", [
            -170.0, -160.0, -150.0, -140.0, -130.0, -120.0, -110.0, -100.0, -90.0,
            -80.0, -70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
            10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0,
            100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0, 180.0
        ]), dtype=np.float64)

    def step(self, observation, reward, done, info,
             agentFullName, observation_space, action_space):
        own = np.asarray(observation["own"], dtype=np.float64)#これらは陣営座標系
        enemy = np.asarray(observation["enemy"], dtype=np.float64)
        threat = np.asarray(observation["threat"], dtype=np.float64)

        numTurn = int(action_space["turn"].n)
        numSpeed = int(action_space["speed"].n)

        # ---- 0. 敵ターゲット切り替えの検知（鋭敏化: 10km -> 4km） ----#モード切替関連の処理。
        if enemy[0] > 0.5:
            az_rad = radians(enemy[1])
            enemy_x = own[0] + enemy[2] * cos(az_rad)#これらは陣営座標系における絶対座標ですよ～
            enemy_y = own[1] + enemy[2] * sin(az_rad)
            
            # 前回位置から 4km 以上飛んでいたらターゲット変更（撃墜等）と判定
            if self.prev_enemy_pos is not None:#前ステップ、
                jump_dist = np.hypot(enemy_x - self.prev_enemy_pos[0], enemy_y - self.prev_enemy_pos[1])
                if jump_dist > 4.0:
                    self.target_locked = False
                    self.disengage_phase = False
                    self.prev_enemy_pos = None #んと、リセットしてるのはターゲット認識yesnoと離脱状況yesnoと、前回敵位置のリセット。

        # ---- 1. ミサイル発射直後の Stage 3（離脱フェーズ）起動処理 ----
        if self.last_action_fired is True:#最上層
            self.target_locked = False
            self.last_action_fired = False
            self.disengage_phase = True #離脱モードアクティベート、前回射撃ダウン、ロックオンダウン。
            self.disengage_step_count = 0
            self.fire_own_pos = (own[0], own[1])  # ★射撃時の自機位置を保持
            
            current_enemy_dist = enemy[2] if enemy[0] > 0.5 else 20.0 #敵との水平距離。見えない場合は20km？
            self.disengage_target_dist = max(current_enemy_dist + 8.0, 30.0)#敵位置から8km以上離脱するか、最低30km離脱するかのどちらか大きい方を離脱目標距離に設定。

       # ---- 2. 向かう方　位[deg]の決定 --------------------------------#基本動作の選択はここ。
        if threat[0] > 0.5:#最上層
            # === 【最優先：MWS警報検知時（回避優先）】 ===
            self.disengage_phase = False #離脱フェーズは中断、回避優先
            speedIndex = numSpeed - 1  # 回避時は全速力

            if self.evasion_phase == "NONE":
                self.evasion_phase = "STAGE1_DIAGONAL"
                dir_to_center = np.array([-own[0], -own[1]])

                az_right = radians(threat[1] + 180.0 + 20.0)
                dir_right = np.array([cos(az_right), sin(az_right)])

                az_left = radians(threat[1] + 180.0 - 20.0)
                dir_left = np.array([cos(az_left), sin(az_left)])

                dot_right = np.dot(dir_right, dir_to_center)
                dot_left = np.dot(dir_left, dir_to_center)

                self.evasion_sign = 1.0 if dot_right >= dot_left else -1.0

            angle_diff = abs((threat[1] + 180.0 - own[4] + 180.0) % 360.0 - 180.0)

            if self.evasion_phase == "STAGE1_DIAGONAL" and angle_diff < 10.0:
                self.evasion_phase = "STAGE2_BEAM"

            if self.evasion_phase == "STAGE1_DIAGONAL":
                desiredAz = threat[1] + 180.0 + (self.evasion_sign * 20.0)
            else:  # STAGE2_BEAM
                desiredAz = threat[1] + 180.0 + (self.evasion_sign * 150.0)

        elif self.disengage_phase: #脅威弾がいない、かつ離脱モードであるとき
            # === 【Stage 3: 射撃後離脱フェーズ】 ===
            self.evasion_phase = "NONE"
            speedIndex = numSpeed - 1  # 振り切るために全速力
            self.disengage_step_count += 1

            # ★射撃位置からの移動距離を計算
            dist_from_fire = np.hypot(own[0] - self.fire_own_pos[0], own[1] - self.fire_own_pos[1])

            # 【完了条件】射撃位置から5km以上移動 OR 4ステップ経過 で即座に再攻撃へ復帰
            if dist_from_fire >= 5.0 or self.disengage_step_count > 1:
                self.disengage_phase = False

            if enemy[0] > 0.5 and self.disengage_phase:
                if enemy[2] >= self.disengage_target_dist:
                    self.disengage_phase = False  # 敵との距離条件クリアで離脱完了
                    desiredAz = enemy[1]
                    speedIndex = numSpeed // 2
                else:
                    desiredAz = enemy[1] + 180.0
            else:
                self.disengage_phase = False
                desiredAz = 0.0
                speedIndex = numSpeed // 2

        else:
            # === 【通常アプローチフェーズ】 ===
            self.evasion_phase = "NONE"

            if enemy[0] > 0.5:
                az_rad = radians(enemy[1])
                enemy_x = own[0] + enemy[2] * cos(az_rad)
                enemy_y = own[1] + enemy[2] * sin(az_rad)

                if not self.target_locked:
                    self.target_locked = True
                    self.lock_start_dist = enemy[2]
                    self.lock_start_own_pos = (own[0], own[1])
                    self.approach_side = 1.0 if enemy_y >= 0.0 else -1.0

                # リードプロディクション
                if self.prev_enemy_pos is not None:
                    dt = 3.0
                    vx_enemy = (enemy_x - self.prev_enemy_pos[0]) / dt
                    vy_enemy = (enemy_y - self.prev_enemy_pos[1]) / dt

                    v_own_kms = own[3] / 1000.0
                    t_intercept = enemy[2] / v_own_kms if v_own_kms > 0 else 0.0
                    t_intercept = min(t_intercept, 10.0)

                    pred_x = enemy_x + vx_enemy * t_intercept
                    pred_y = enemy_y + vy_enemy * t_intercept

                    dx = pred_x - own[0]
                    dy = pred_y - own[1]
                    base_desiredAz = degrees(atan2(dy, dx))
                else:
                    base_desiredAz = enemy[1]

                self.prev_enemy_pos = (enemy_x, enemy_y)

                # 迂回アプローチ
                dist_traveled = np.hypot(own[0] - self.lock_start_own_pos[0],
                                         own[1] - self.lock_start_own_pos[1])

                if dist_traveled < (0.4 * self.lock_start_dist):
                    offset_angle = 10.0 * self.approach_side 
                else:
                    offset_angle = 0.0

                desiredAz = base_desiredAz + offset_angle
                speedIndex = numSpeed // 2

            else:
                self.target_locked = False
                self.prev_enemy_pos = None
                desiredAz = 0.0
                speedIndex = numSpeed // 2

        # ---- 3. 場外回避（最優先） ------------------------------------
        marginSide, marginFront, marginBack = own[5], own[6], own[7]
        if marginFront < self.marginKm:
            desiredAz = 100.0 if own[1] < 0.0 else -100.0
        elif marginBack < self.marginKm:
            desiredAz = 0.0
        if marginSide < self.marginKm:
            desiredAz = -60.0 if own[1] > 0.0 else 60.0

        # ---- 4. インデックス選定 --------------------------------------
        turnIndex = self._nearestTurnIndex(desiredAz, numTurn)

        # ---- 5. 射撃判定 ----------------------------------------------
        targetIndex = 0
        if (enemy[0] > 0.5) and (enemy[2] <= self.fireRangeKm) and (threat[0] < 0.5):
            targetIndex = 1
            self.last_action_fired = True

        return {"turn": turnIndex, "speed": speedIndex, "target": targetIndex}

    def _nearestTurnIndex(self, desiredAz, numTurn):
        """turnTable の中から desiredAz に一番近い角度の index を動的に返す"""
        diffs = (np.array(self.turnTable) - desiredAz + 180.0) % 360.0 - 180.0
        return int(np.argmin(np.abs(diffs)))