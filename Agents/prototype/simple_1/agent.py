"""セミナー用の最小構成エージェント (S方式: 1機に1エージェント)

このファイルは「Agentクラスに何を実装すればよいか」を理解するための教材である。
実装しているのは以下の7メソッドのみ。

    initialize        : 設定(modelConfig)の読み込み。インスタンス生成時に1回だけ呼ばれる
    validate          : 他の登場物(Ruler等)に依存する初期化。シミュレーション開始時に1回だけ
    makeObs           : observables(生の観測情報) -> Observation(行動判断用に加工した情報)
    observation_space : makeObs が返す Observation の型
    action_space      : policy が返す action の型
    deploy            : action を解釈して「どう動きたいか」を決める。1step(既定3秒)に1回
    control           : 決めた内容を commands(機体への指令)として出す。1tick(1秒)ごと

perceive / behave / serializeInternalState は必須ではないため実装していない。
より細かい制御や網羅的な解説は make_agent.ipynb と
simulator/sample/modules/r8_contest_sample/ を参照すること。

【重要】commands は control で出すこと
    基底クラスの Agent::control() は、毎 tick すべての parent の commands を
    空オブジェクトにリセットする(simulator/src/Agent.cpp を参照)。
    deploy は 1 step (既定3秒) に1回しか呼ばれないため、deploy でしか commands を
    書かないと、次の tick で消されてしまい機体側が指令を読めずエラーになる。
    そのため commands は 1 tick ごとに呼ばれる control で出す。
    (control を `pass` でオーバーライドしてリセットを止める書き方もあるが、
     その場合は射撃フラグが立ちっぱなしになるなどの副作用に注意が必要。)

【重要】このエージェントは学習しない(ルールベース)ので、Observationは正規化せず
物理量そのまま(km, m/s, degree)にしている。見たままの値なので policy が読みやすい。
ニューラルネットに入力する場合は正規化が必要になる(→ tutorial/RLAgent/ を参照)。
"""

import numpy as np
from math import atan2, cos, sin, sqrt, degrees, radians
from gymnasium import spaces

from ASRCAISim1.core import (  # type: ignore
    SingleAssetAgent,
    MotionState,
    Track3D,
    Track2D,
    AltitudeKeeper,
    getValueFromJsonKRD,
)
from BasicAgentUtility.util import TeamOrigin, sortTrack3DByDistance  # type: ignore


class SimpleAgent(SingleAssetAgent):
    """最近傍の彼機に向かい、距離が射程内なら撃つ。被弾警報が出たら背を向けて逃げる。"""

    # ------------------------------------------------------------------
    # 1) initialize : modelConfig から設定を読む
    # ------------------------------------------------------------------
    def initialize(self):
        # 基底クラスの initialize は必ず呼ぶこと
        super().initialize()

        # 自陣営("Red" または "Blue")
        self.own = self.getTeam()

        # modelConfig は config.json の内容。getValueFromJsonKRD は
        # 「キーが無ければ第4引数の既定値を使う」という読み出しユーティリティ。
        mc = self.modelConfig

        # --- 行動の選択肢 ---
        # 目標方位[deg]の選択肢。自陣営の進行方向(敵陣方向)を0度とした方位角で、右が正。
        self.turnTable = np.array(
            getValueFromJsonKRD(mc, "turnTable", self.randomGen,
                                [-180.0, -135.0, -90.0, -45.0, 0.0, 45.0, 90.0, 135.0]),
            dtype=np.float64)
        # 目標速度[m/s]の選択肢
        self.speedTable = np.array(
            getValueFromJsonKRD(mc, "speedTable", self.randomGen, [530.0, 550.0, 570.0]),
            dtype=np.float64)
        # 射撃目標として選べる彼機の数(action の target は 0=撃たない, 1..maxEnemyNum)
        self.maxEnemyNum = int(getValueFromJsonKRD(mc, "maxEnemyNum", self.randomGen, 5))

        # --- 飛行の制限 ---
        self.altMin = getValueFromJsonKRD(mc, "altMin", self.randomGen, 2000.0)
        self.altMax = getValueFromJsonKRD(mc, "altMax", self.randomGen, 15000.0)
        self.maxSimulShot = int(getValueFromJsonKRD(mc, "maxSimulShot", self.randomGen, 2))
        # 目標高度に向けてピッチ角を計算してくれるユーティリティ
        self.altitudeKeeper = AltitudeKeeper(mc().get("altitudeKeeper", {}))

        # --- makeObs / deploy / control の間で受け渡す情報 ---
        # 探知中の彼機航跡。makeObs で作り、deploy で射撃目標の指定に使う。
        self.lastTrackInfo = []
        # deploy が決めて control が commands として出す内容
        self.dstDir = np.array([1.0, 0.0, 0.0])   # 目標進行方向(慣性座標系)
        self.dstV = float(self.speedTable[len(self.speedTable) // 2])  # 目標速度
        self.launchFlag = False                   # 射撃するか
        self.target = Track3D()                   # 射撃目標

    # ------------------------------------------------------------------
    # 2) validate : 他の登場物に依存する初期化
    # ------------------------------------------------------------------
    def validate(self):
        # Ruler(ルール判定を行う登場物)から戦域の寸法を取得する。
        # 末尾の () で nljson から Python の dict に変換している。
        rulerObs = self.manager.getRuler()().observables()
        self.dOut = rulerObs["dOut"]    # 戦域中心から場外ラインまでの距離[m]
        self.dLine = rulerObs["dLine"]  # 戦域中心から防衛ラインまでの距離[m]

        # 陣営座標系(自陣営の進行方向が+x、防衛ライン中央が原点)への変換クラス。
        # これを使うと Red でも Blue でも同じロジックで書けるようになる。
        self.teamOrigin = TeamOrigin(self.own == rulerObs["eastSider"], self.dLine)

        # 初期の目標進行方向は「敵陣方向にまっすぐ」としておく
        self.dstDir = self.teamOrigin.relBtoP(np.array([1.0, 0.0, 0.0]))

    # ------------------------------------------------------------------
    # 3) makeObs : observables -> Observation
    # ------------------------------------------------------------------
    def makeObs(self):
        parent = self.parent  # SingleAssetAgent は自分が担当する1機を self.parent で参照する

        own = np.zeros([8], dtype=np.float32)
        enemy = np.zeros([4], dtype=np.float32)
        threat = np.zeros([2], dtype=np.float32)
        self.lastTrackInfo = []

        if not parent.isAlive():
            # 撃墜されている場合、observables はもう更新されないので何も見えない
            return {"own": own, "enemy": enemy, "threat": threat}

        # --- 自機の運動状態 ---
        # observables["motion"] という json から MotionState オブジェクトを復元する。
        # transformTo(self.getLocalCRS()) は行動判断用の座標系(NED)に揃えるための変換。
        myMotion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())
        # relPtoB は「回転のみ」で平行移動を含まない(平行移動込みは absPtoB)。
        # ここでは公式サンプルと同じく戦域中心を原点とした陣営座標系で扱う。
        pos = self.teamOrigin.relPtoB(myMotion.pos())  # 慣性座標系 -> 陣営座標系
        vel = self.teamOrigin.relPtoB(myMotion.vel())
        V = float(np.linalg.norm(vel))
        own[0] = pos[0] / 1000.0    # 敵陣方向の位置[km] (戦域中心基準。正なら敵陣側) #これは陣営座標系です！！！
        own[1] = pos[1] / 1000.0    # 横方向の位置[km] (戦域中心基準)
        own[2] = -pos[2] / 1000.0   # 高度[km] (陣営座標系はz下向きなので符号を反転)
        own[3] = V                  # 速さ[m/s]
        own[4] = degrees(atan2(vel[1], vel[0])) if V > 1e-5 else 0.0  # 進行方位[deg]

        # 場外ラインまでの余裕[km]。場外に長く留まると罰則が累積し、
        # 一定量を超えると失格(endReason が PENALTY)になって負けるため、
        # 「あとどれだけ余裕があるか」を Policy に伝えておく。
        own[5] = (self.dOut - abs(pos[1])) / 1000.0   # 横方向の余裕
        own[6] = (self.dLine - pos[0]) / 1000.0       # 前方(敵陣側)の余裕
        own[7] = (self.dLine + pos[0]) / 1000.0       # 後方(自陣側)の余裕

        # --- 彼機の航跡 ---
        # 味方全機のレーダ航跡はマージされて /sensor/track に入っている。
        # json から Track3D オブジェクトを復元し、自機に近い順に並べる。
        self.lastTrackInfo = [
            Track3D(t).transformTo(self.getLocalCRS())
            for t in parent.observables.at_p("/sensor/track")
        ]
        sortTrack3DByDistance(self.lastTrackInfo, [myMotion], True)

        if len(self.lastTrackInfo) > 0:
            track = self.lastTrackInfo[0]  # 最も近い彼機
            tPos = self.teamOrigin.relPtoB(track.pos())
            rel = tPos - pos #自機との相対ベクトル、陣営座標系
            enemy[0] = 1.0                                        # 探知しているか
            enemy[1] = degrees(atan2(rel[1], rel[0]))             # 彼機の方位[deg]
            enemy[2] = float(np.linalg.norm(rel[:2])) / 1000.0    # 水平距離[km]
            enemy[3] = -tPos[2] / 1000.0                          # 彼機の高度[km]

        # --- 被弾警報(MWS: Missile Warning System) ---
        # 検出できるのは誘導弾が飛んで来ている「方向」だけで、距離は分からない。
        if parent.observables.contains_p("/sensor/mws/track"):
            for mObs in parent.observables.at_p("/sensor/mws/track"):
                mwsTrack = Track2D(mObs).transformTo(self.getLocalCRS())
                d = self.teamOrigin.relPtoB(mwsTrack.dir())
                threat[0] = 1.0
                threat[1] = degrees(atan2(d[1], d[0]))  # 誘導弾が居る方位[deg]
                break  # 教材なので最初の1発だけ見る

        # 戻り値は対戦ログに保存される。numpy.ndarray は自動で list に変換されるため
        # そのまま返してよい(json 化できない型を混ぜるとログが残らなくなる)。
        return {"own": own, "enemy": enemy, "threat": threat}

    # ------------------------------------------------------------------
    # 4) observation_space : makeObs が返す Observation の型
    # ------------------------------------------------------------------
    def observation_space(self):
        low = np.finfo(np.float32).min
        high = np.finfo(np.float32).max
        return spaces.Dict({  # type: ignore
            "own": spaces.Box(low, high, shape=[8], dtype=np.float32),
            "enemy": spaces.Box(low, high, shape=[4], dtype=np.float32),
            "threat": spaces.Box(low, high, shape=[2], dtype=np.float32),
        })

    # ------------------------------------------------------------------
    # 5) action_space : policy が返す action の型
    # ------------------------------------------------------------------
    def action_space(self):
        # S方式(1機に1エージェント)なので、1機分の行動をそのまま定義すればよい。
        # 編隊全体を1つのエージェントで操作するM方式の場合は、これを機数分並べた
        # spaces.Tuple にする必要がある(→ tutorial/RLAgent/ を参照)。
        return spaces.Dict({  # type: ignore
            "turn": spaces.Discrete(len(self.turnTable)),   # どの方位に向かうか
            "speed": spaces.Discrete(len(self.speedTable)),  # どの速度で飛ぶか
            "target": spaces.Discrete(1 + self.maxEnemyNum),  # 0=撃たない, 1..=何番目の彼機を撃つか
        })

    # ------------------------------------------------------------------
    # 6) deploy : action を解釈して「どう動きたいか」を決める(1 step ごと)
    #
    # ここでは commands を書かない。commands は毎 tick 呼ばれる control で出す。
    # (基底クラスの Agent::control() が毎 tick commands をリセットするため)
    # ------------------------------------------------------------------
    def deploy(self, action):
        parent = self.parent
        if not parent.isAlive():
            return

        # --- (a) 目標進行方向 ---
        # 陣営座標系での方位角を、慣性座標系のベクトルに変換する。
        az = radians(float(self.turnTable[int(action["turn"])]))
        self.dstDir = self.teamOrigin.relBtoP(np.array([cos(az), sin(az), 0.0]))

        # --- (b) 目標速度 ---
        self.dstV = float(self.speedTable[int(action["speed"])])

        # --- (c) 射撃の可否と目標 ---
        self.launchFlag = False
        self.target = Track3D()
        tIdx = int(action["target"]) - 1  # 0 は「撃たない」なので1つずらす
        if 0 <= tIdx < len(self.lastTrackInfo):
            candidate = self.lastTrackInfo[tIdx]
            # isLaunchableAt: その航跡に対して今撃てるか(射程・センサ条件等)を機体が判定する
            if parent.isLaunchableAt(candidate) and self._countFlyingMissiles() < self.maxSimulShot:
                self.launchFlag = True
                self.target = candidate

    # ------------------------------------------------------------------
    # 7) control : commands(機体への指令)を出す(1 tick ごと)
    # ------------------------------------------------------------------
    def control(self):
        parent = self.parent
        if not parent.isAlive():
            return

        myMotion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())

        # --- 高度が上下限に収まるようにピッチ角を補正する ---
        # 毎 tick 計算し直すので、機体の現在高度に応じた滑らかな上昇・下降になる。
        dstDir = np.array(self.dstDir, dtype=np.float64)
        n = sqrt(dstDir[0] * dstDir[0] + dstDir[1] * dstDir[1])
        if n < 1e-8:
            # 真上/真下を向いてしまっている場合の保険
            dstDir = self.teamOrigin.relBtoP(np.array([1.0, 0.0, 0.0]))
            n = sqrt(dstDir[0] * dstDir[0] + dstDir[1] * dstDir[1])
        dstPitch = atan2(-dstDir[2], n)
        bottom = self.altitudeKeeper(myMotion, dstDir, self.altMin)
        minPitch = atan2(-bottom[2], sqrt(bottom[0] * bottom[0] + bottom[1] * bottom[1]))
        top = self.altitudeKeeper(myMotion, dstDir, self.altMax)
        maxPitch = atan2(-top[2], sqrt(top[0] * top[0] + top[1] * top[1]))
        dstPitch = max(minPitch, min(maxPitch, dstPitch))
        cs = cos(dstPitch)
        sn = sin(dstPitch)
        dstDir = np.array([dstDir[0] / n * cs, dstDir[1] / n * cs, -sn])
        # --- commands を書き込む ---
        # ここまでの計算は行動判断用の座標系(localCRS)で行ってきたので、
        # 機体に渡す際は機体側の座標系に戻す必要がある。
        originalMyMotion = MotionState(parent.observables["motion"])
        self.commands[parent.getFullName()] = {
            "motion": {
                "dstDir": originalMyMotion.dirAtoP(dstDir, myMotion.pos(), self.getLocalCRS()),
                "dstV": self.dstV,
            },
            "weapon": {
                "launch": self.launchFlag,
                "target": self.target.to_json(),
            },
        }

        # 射撃は1回だけ。立てたままにすると毎 tick 射撃要求を出し続けてしまう。
        self.launchFlag = False

    # ------------------------------------------------------------------
    # 以下はフレームワークのメソッドではなく、このクラス内部の補助関数
    # ------------------------------------------------------------------
    def _countFlyingMissiles(self):
        """自機が発射済みでまだ飛翔中の誘導弾の数を数える。"""
        count = 0
        if self.parent.observables.contains_p("/weapon/missiles"):
            for msl in self.parent.observables.at_p("/weapon/missiles"):
                if msl.at("isAlive")() and msl.at("hasLaunched")():
                    count += 1
        return count
