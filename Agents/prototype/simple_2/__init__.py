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
        self.fireRangeKm = float(args.get("fireRangeKm", 60.0))
        self.marginKm = float(args.get("marginKm", 25.0))

        # 状態管理変数
        self.disengage_phase = 0  # 4step退避中かどうか
        self.disengage_step_count = 0  # 退避ステップ数カウンター
        self.now_step = 0
        self.prev_enemy_pos = None  # 前ステップの敵位置（絶対座標）[km]。初期値は None
        print("Hallo world!")

        # turnTable（10度刻み・全36方位）
        self.turnTable = np.array(args.get("turnTable", [
                    -180, -170.0, -160.0, -150.0, -140.0, -130.0, -120.0, -110.0, -100.0, -90.0,
                    -80.0, -70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
                    10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0,
                    100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0,
                ]), dtype=np.float64)
        self.penalty = 0

    def reset(self):
        """エピソード開始時に呼ぶリセット関数"""
        self.disengage_phase = 0
        self.disengage_step_count = 0
        self.penalty = 0

    def _normalize_angle(self, angle):
        """角度を [-180, 180) に正規化"""
        return (angle + 180.0) % 360.0 - 180.0

    def _nearestTurnIndex(self, desiredAz):
        """180/-180度の境界をまたいでも正確に最短インデックスを返す"""
        desiredAz = self._normalize_angle(desiredAz)
        diffs = (self.turnTable - desiredAz + 180.0) % 360.0 - 180.0
        return int(np.argmin(np.abs(diffs)))

    def step(
        self,
        observation,
        reward,
        done,
        info,
        agentFullName,
        observation_space,
        action_space,
    ):
        own = np.asarray(observation["own"], dtype=np.float64)
        enemy = np.asarray(observation["enemy"], dtype=np.float64)
        threat = np.asarray(observation["threat"], dtype=np.float64)

        numSpeed = int(action_space["speed"].n)

        # デフォルト値
        desiredAz = 0.0
        target_label = 0
        speedIndex = numSpeed // 2  # 通常時は中速
        targetIndex = 0
        self.now_step += 1
        enemy_heading = None  # 敵の進行方向[deg] (-180 ~ 180)
        heading_diff = None
        self.penalty -= 10 if self.penalty > 0 else 0 # ペナルティは徐々に減衰

        
        if self.now_step <= 1:
            print(f"I'm {own[9]}th agent.")
            print("ypaaaaaa")
        if enemy[0] > 0.5: # 敵が見えている場合、敵の絶対座標を計算して進行方向と角度差を算出
            az_rad = radians(enemy[1])
            # 敵の絶対座標（陣営座標系 [km]）
            enemy_x = own[0] + enemy[2] * cos(az_rad)
            enemy_y = own[1] + enemy[2] * sin(az_rad)

            # 前ステップの位置が存在する場合、進行方向と角度差を計算
            if self.prev_enemy_pos is not None: 
                dx_enemy = enemy_x - self.prev_enemy_pos[0]
                dy_enemy = enemy_y - self.prev_enemy_pos[1]
                dist_moved = np.hypot(dx_enemy, dy_enemy)
                if dist_moved < 0.27 and dist_moved > 0.23:  # 標的が輸送機であるかどうか。target_label = 1で輸送機
                    target_label = 1
                else:
                    target_label = 0

                # ターゲット急変（撃墜等で20km以上飛んだ場合）の判定
                if dist_moved > 20.0:
                    self.prev_enemy_pos = None
                elif dist_moved > 0.001:  # 微小な移動がある場合のみ進行方向を算出
                    # 1. 敵の進行方向[deg] (-180° 〜 +180°)
                    enemy_heading = degrees(atan2(dy_enemy, dx_enemy))

                    # 2. 自機の進行方向[deg] (own[4] に格納されている)
                    own_heading = enemy[1]  # own[4] は自機の進行方向[deg] (-180° 〜 +180°)

                    # 3. 2つの進行方向ベクトルのなす角度差[deg] (0°: 同方向平行 〜 180°: 正向かい/反航)
                    raw_diff = enemy_heading - own_heading
                    heading_diff = abs((raw_diff + 180.0) % 360.0 - 180.0)
            else:
                # 前ステップの位置が存在しない場合、進行方向は不明
                enemy_heading = 0.0
                heading_diff = None

            # 次ステップ用に現在位置を更新
            self.prev_enemy_pos = (enemy_x, enemy_y)
        else:
            self.prev_enemy_pos = None


        self.fireRangeKm = 55.0 if self.now_step < 600 else 70.0  # 100stepまでは射程50km、以降は80km
        if heading_diff is not None: #敵との偏角により射程距離を調整
            if heading_diff <= 90.0:
                self.fireRangeKm_ad = self.fireRangeKm + 20.0 * sin(radians(heading_diff))*-1
            else:
                self.fireRangeKm_ad = self.fireRangeKm + 20.0 * sin(radians(heading_diff)) 
        else:
            self.fireRangeKm_ad = self.fireRangeKm

        if target_label == 1:#輸送機標的の時、射程を短くして確実に当てる。
            self.fireRangeKm_ad = self.fireRangeKm_ad - 20 

        # =========================================================================
        # 飛行方針の決定（下に行くほど優先度が高い）
        # ================================  =========================================

        # 【① 通常アプローチ】基本的に敵へ直進（見えない場合は正面0度）
        upper_or_lower = int(own[1] / abs(own[1])) if own[1] != 0 else -1

        if enemy[0] > 0.5: #通常直進。やや迂回して接近
            if enemy[2] >= (self.fireRangeKm_ad + 20.0):#射程距離+30kmまでは20°迂回しながら接近させる。
                desiredAz = enemy[1] + 10.0 * upper_or_lower * (abs(2.5 - own[9]))
            else:            
                desiredAz = enemy[1] + 0.0 * upper_or_lower * (abs(2.5 - own[9]))
            self.penalty += 20 if enemy[2] <= self.fireRangeKm_ad + 80 and target_label == 0 else 0 #通常直進ペナルティ
        else:
            desiredAz = 0.0

        # ペナルティアップ時、しばらくは敵と逆方向へ全速離脱
        if self.disengage_phase >= 1:
            self.penalty = 0  # ペナルティアップ時、penaltyは常にリセット
            self.disengage_step_count += 1
            speedIndex = numSpeed - 1 if self.disengage_phase == 2 else numSpeed // 2 # phase1退避時は低速　phase2退避時は全速力
            
            # 1. 基本の離脱方位（敵の反対、または自陣方向180°）
            desiredAz = self._normalize_angle(enemy[1] + 180.0 if enemy[0] > 0.5 else 180.0)

            # 離脱ステップ数の動的計算
            basic_limit = 70.0 if self.disengage_phase == 1 else 90.0
            
            if heading_diff is not None:
                disengage_limit = basic_limit - 10.0 * self.Then_heading_diff
            else:
                disengage_limit = basic_limit

            if self.disengage_step_count >= disengage_limit and enemy[2] > (self.fireRangeKm-30) + 40 * self.disengage_phase:
                self.disengage_phase = 0
                self.disengage_step_count = 0
                print(f"{self.now_step}秒時点、離脱終了")
            if self.disengage_step_count == 1:
                print(f"{self.now_step}秒時点、離脱開始。標的角度{enemy[1]}、敵進行角度{enemy_heading}、離脱時間{disengage_limit}秒。射撃角度偏差{heading_diff}度。")



        elif self.penalty >= 100 and self.penalty < 120:
            self.disengage_phase = 1
            self.disengage_step_count = 0
            self.Then_heading_diff = cos(radians(heading_diff) if heading_diff is not None else 0.0)
        elif self.penalty >= 120:
            self.disengage_phase = 2
            self.disengage_step_count = 0
            self.Then_heading_diff = cos(radians(heading_diff) if heading_diff is not None else 0.0)
            

        # 【④ MWS警報回避】ミサイルから180°反転して全速退避（退避フェーズもキャンセル）
        if threat[0] > 0.5:
            self.disengage_phase = 0
            self.penalty = 0
            #desiredAz = threat[1] + 180.0
            desiredAz = self._normalize_angle(enemy[1] + 180.0 if enemy[0] > 0.5 else 180.0)
            speedIndex = numSpeed - 1  # 回避時は全速力
            print(f"{self.now_step}秒時点、MWS警報。")


        # 【⑤ エリア外防止（最優先）】場外ラインが近づいたらエリア中央へ回頭
        marginSide, marginFront, marginBack = own[5], own[6], own[7]
        if marginFront < self.marginKm:
            desiredAz = 100.0 if own[1] < 0.0 else -100.0
            speedIndex = numSpeed - 1
        elif marginBack < self.marginKm:
            desiredAz = 0.0
            speedIndex = numSpeed - 1
        elif marginSide < self.marginKm:
            desiredAz = -60.0 if own[1] > 0.0 else 60.0
            speedIndex = numSpeed - 1

        #外苑に近づいたとき、ペナルティをリセットして、離脱フェーズをキャンセルする。
        if marginFront < self.marginKm + 30 or marginBack < self.marginKm + 30 or marginSide < self.marginKm + 30:
            self.penalty = 0
            self.disengage_phase = 0

        if enemy[4] < 4:
            self.penalty = 0
            self.disengage_phase = 0


        # =========================================================================
        # 行動インデックス選定 & 射撃判定
        # =========================================================================

        # 1. 目標角度を旋回インデックスに変換
        turnIndex = self._nearestTurnIndex(desiredAz)

        # 2. 【② 射撃判定】敵が射程内 & MWS警報なし & 離脱中でなければ発砲
        if (
            (enemy[0] > 0.5)
            and (enemy[2] <= self.fireRangeKm_ad)
            and (threat[0] < 0.5)
            and (self.disengage_phase == 0)
        ):
            targetIndex = 1
            # 射撃申請

        if own[8] == 1:#実際に発砲、りだつふぇーずへ　
            self.penalty += 150
            print(f"{self.now_step}秒時点、発砲。")




        return {"turn": turnIndex, "speed": speedIndex, "target": targetIndex}