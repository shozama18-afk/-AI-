"""提出フォーマットの実体 (S方式: 1機に1エージェント)

対戦プログラムはこのディレクトリを Python モジュールとして import し、
以下の4つの関数を呼ぶ。この4つが揃っていることが提出の必須条件である。

    getUserAgentClass       : 使用する Agent クラスを返す
    getUserAgentModelConfig : Agent に渡す設定(modelConfig)を返す
    isUserAgentSingleAsset  : 1機に1エージェント(True)か、編隊全体で1エージェント(False)か
    getUserPolicy           : Observation から action を決める Policy を返す

いずれの関数も引数 args を取り、args.json の内容が渡される。
"""

import json
import os

import numpy as np

from ASRCAISim1.policy import StandalonePolicy  # type: ignore


# ======================================================================
# 提出に必須の4関数
# ======================================================================

def getUserAgentClass(args={}):
    """使用する Agent クラスを返す。"""
    from .agent import SimpleAgent
    return SimpleAgent


def getUserAgentModelConfig(args={}):
    """Agent に渡す設定を返す。ここでは config.json をそのまま渡している。"""
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
    """学習を使わないルールベースの Policy。

    SimpleAgent.makeObs() が返す Observation は物理量そのまま(km, m/s, degree)
    なので、以下のように「見たまま」の条件分岐で書ける。

        1. 被弾警報が出ていたら、その方向に背を向けて逃げる
        2. 彼機を探知していたら、その方向へ向かい、射程内なら撃つ
        3. 何も見えていなければ、敵陣方向(方位0度)へ前進する

    ただし横方向に出過ぎると場外になるため、戦域中央側へ戻す補正を入れている。
    """

    def __init__(self, args={}):
        super().__init__()#■親クラスのあれこれ初期化をアクティベート
        args = args if args is not None else {}
        # args.json から調整できるようにしておく。
        # fireRangeKm は「この距離以内なら射撃を要求する」しきい値。
        # 実際に撃てるかどうかは Agent 側で parent.isLaunchableAt() が判定するので、
        # 大きめの値にして機体の判定に任せるのがよい(小さすぎると近づく前に撃墜される)。
        self.fireRangeKm = float(args.get("fireRangeKm", 80.0))
        # 場外ラインまでの余裕がこの値[km]を下回ったら戦域中央側へ戻す
        self.marginKm = float(args.get("marginKm", 25.0))

    def step(self, observation, reward, done, info,
             agentFullName, observation_space, action_space):
        own = np.asarray(observation["own"], dtype=np.float64)
        enemy = np.asarray(observation["enemy"], dtype=np.float64)
        threat = np.asarray(observation["threat"], dtype=np.float64)

        numTurn = int(action_space["turn"].n)
        numSpeed = int(action_space["speed"].n)

        # ---- 向かう方位[deg]を決める --------------------------------
        if threat[0] > 0.5:
            # 1. 誘導弾が飛んで来ている: その方位に背を向ける
            desiredAz = threat[1] + 180.0
            speedIndex = numSpeed - 1  # 全速で振り切る
        elif enemy[0] > 0.5:
            # 2. 彼機を探知している: その方位へ向かう
            desiredAz = enemy[1]
            speedIndex = numSpeed // 2#■標準速度
        else:
            # 3. 何も見えていない: 敵陣方向へ前進する
            desiredAz = 0.0
            speedIndex = numSpeed // 2#■標準速度

        # ---- 場外を避ける(最優先) ------------------------------------
        # 場外に留まると罰則が累積し、一定量を超えると失格して負けるため、
        # 上で決めた方位を上書きして戦域中央側へ戻す。
        #   own[5] = 横方向の余裕[km], own[6] = 前方の余裕, own[7] = 後方の余裕
        marginSide, marginFront, marginBack = own[5], own[6], own[7]
        if marginFront < self.marginKm:
            # 敵陣側に出過ぎている: 横に流して前進を止める
            desiredAz = 90.0 if own[1] < 0.0 else -90.0
        elif marginBack < self.marginKm:
            # 自陣側に下がり過ぎている: 敵陣方向へ戻す
            desiredAz = 0.0
        if marginSide < self.marginKm:
            # 横に出過ぎている: 中央側へ寄せながら前進する
            desiredAz = -60.0 if own[1] > 0.0 else 60.0

        # ---- 方位の選択肢のうち、最も desiredAz に近いものを選ぶ ----
        turnIndex = self._nearestTurnIndex(desiredAz, numTurn)

        # ---- 射撃するかどうか ---------------------------------------
        # 0 は「撃たない」。1 は「最も近い彼機を撃つ」。
        targetIndex = 0
        if enemy[0] > 0.5 and enemy[2] <= self.fireRangeKm and threat[0] < 0.5:
            targetIndex = 1#makeobsでやってた。lasttrackなんちゃらは自機から近い順に計算

        return {"turn": turnIndex, "speed": speedIndex, "target": targetIndex}

    def _nearestTurnIndex(self, desiredAz, numTurn):
        """turnTable の選択肢のうち desiredAz に角度が最も近いものの index を返す。

        Policy 側は turnTable の中身を知らないので、選択肢が
        [-180, -135, -90, -45, 0, 45, 90, 135] 度で等間隔に並んでいると仮定して
        index を逆算している(config.json の turnTable を変える場合はここも見直すこと)。
        """
        # -180 <= az < 180 に正規化
        az = (desiredAz + 180.0) % 360.0 - 180.0
        step = 360.0 / numTurn
        index = int(round((az + 180.0) / step)) % numTurn
        return index
    #def _nearestTurnIndex(self, desiredAz):
        """turnTable の中から desiredAz に一番近い角度の index を動的に返す"""
        # 360度の周回（-180度と180度の境界）を考慮した角度差を計算
        diffs = (np.array(self.turnTable) - desiredAz + 180.0) % 360.0 - 180.0
        
        # 角度差の絶対値（エラー量）が最小になるインデックスを一発で取得
        return int(np.argmin(np.abs(diffs)))