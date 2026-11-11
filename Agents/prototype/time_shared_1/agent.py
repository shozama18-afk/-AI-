"""時間差ウェーブ・VIP狙い・自機残弾管理対応エージェント"""

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
    """自機残弾数(own[10])取得および最奥敵ターゲット選択対応エージェント"""

    def assign_role_id(self):
        """初期配置の y 座標が大きい順に 1〜4 の固有IDを判定・保持する"""
        if getattr(self, "my_role_id", None) is not None:
            return self.my_role_id

        parent = self.parent
        if not parent.isAlive():
            return None

        if not parent.observables.contains_p("/shared/fighter"):
            return None

        shared_fighters = parent.observables.at_p("/shared/fighter")
        if shared_fighters is None:
            return None

        fighter_positions = []
        my_name = parent.getFullName()
        my_motion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())
        my_pos = self.teamOrigin.relPtoB(my_motion.pos())
        fighter_positions.append((my_name, my_pos[1]))

        for fullName, fObs in shared_fighters.items():
            if fObs.contains_p("/sensor/radar"):
                motion = MotionState(fObs["motion"]).transformTo(self.getLocalCRS())
                pos = self.teamOrigin.relPtoB(motion.pos())
                y_pos = pos[1]
                fighter_positions.append((fullName, y_pos))

        if len(fighter_positions) == 0:
            return None

        fighter_positions.sort(key=lambda x: x[1], reverse=True)

        my_name = parent.getFullName()
        for rank, (name, _) in enumerate(fighter_positions, start=1):
            if name == my_name:
                self.my_role_id = rank
                print(f"[{my_name}] 固有ID割り当て完了: Role ID {self.my_role_id}")
                break

        return self.my_role_id

    def initialize(self):
        super().initialize()

        self.own = self.getTeam()
        self.my_role_id = None

        mc = self.modelConfig

        self.turnTable = np.array(
            getValueFromJsonKRD(mc, "turnTable", self.randomGen,
                                 [-180, -170.0, -160.0, -150.0, -140.0, -130.0, -120.0, -110.0, -100.0, -90.0, -80.0, -70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0]),
            dtype=np.float64)
        self.speedTable = np.array(
            getValueFromJsonKRD(mc, "speedTable", self.randomGen, [530.0, 550.0, 570.0]),
            dtype=np.float64)
        self.maxEnemyNum = int(getValueFromJsonKRD(mc, "maxEnemyNum", self.randomGen, 5))

        self.altMin = getValueFromJsonKRD(mc, "altMin", self.randomGen, 2000.0)
        self.altMax = getValueFromJsonKRD(mc, "altMax", self.randomGen, 15000.0)
        self.maxSimulShot = int(getValueFromJsonKRD(mc, "maxSimulShot", self.randomGen, 2))
        self.altitudeKeeper = AltitudeKeeper(mc().get("altitudeKeeper", {}))

        self.lastTrackInfo = []
        self.dstDir = np.array([1.0, 0.0, 0.0])
        self.dstV = float(self.speedTable[len(self.speedTable) // 2])
        self.launchFlag = False
        self.target = Track3D()
        self.fired_log = 0

    def validate(self):
        parent = self.parent
        rulerObs = self.manager.getRuler()().observables()
        self.dOut = rulerObs["dOut"]
        self.dLine = rulerObs["dLine"]

        self.teamOrigin = TeamOrigin(self.own == rulerObs["eastSider"], self.dLine)
        self.dstDir = self.teamOrigin.relBtoP(np.array([1.0, 0.0, 0.0]))

    def makeObs(self):
        parent = self.parent

        own = np.zeros([11], dtype=np.float32)
        enemy = np.zeros([5], dtype=np.float32)
        threat = np.zeros([2], dtype=np.float32)
        self.lastTrackInfo = []

        if not parent.isAlive():
            return {"own": own, "enemy": enemy, "threat": threat}

        # --- 自機の運動状態 ---
        myMotion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())
        pos = self.teamOrigin.relPtoB(myMotion.pos())
        vel = self.teamOrigin.relPtoB(myMotion.vel())
        V = float(np.linalg.norm(vel))
        own[0] = pos[0] / 1000.0    # 敵陣方向の位置[km]
        own[1] = pos[1] / 1000.0    # 横方向の位置[km]
        own[2] = -pos[2] / 1000.0   # 高度[km]
        own[3] = V                  # 速さ[m/s]
        own[4] = degrees(atan2(vel[1], vel[0])) if V > 1e-5 else 0.0

        own[5] = (self.dOut - abs(pos[1])) / 1000.0   # 横方向の余裕
        own[6] = (self.dLine - pos[0]) / 1000.0       # 前方(敵陣側)の余裕
        own[7] = (self.dLine + pos[0]) / 1000.0       # 後方(自陣側)の余裕
        own[8] = self.fired_log

        ally_tag = self.assign_role_id()
        own[9] = ally_tag if ally_tag is not None else 1

        # ★ 修正: 正しい残弾数パス /weapon/remMsls の参照[cite: 3] ★
        if parent.observables.contains_p("/weapon/remMsls"):
            own[10] = float(parent.observables.at_p("/weapon/remMsls"))
        else:
            own[10] = 0.0

        # --- 彼機の航跡 ---
        self.lastTrackInfo = [
            Track3D(t).transformTo(self.getLocalCRS())
            for t in parent.observables.at_p("/sensor/track")
        ]

        # 前方余裕 own[6] が 50km 未満なら最奥敵（VIP）を優先ソート
        if own[6] < 50.0 and len(self.lastTrackInfo) > 0:
            self.lastTrackInfo.sort(
                key=lambda track: self.teamOrigin.relPtoB(track.pos())[0],
                reverse=True
            )
        else:
            sortTrack3DByDistance(self.lastTrackInfo, [myMotion], True)

        if len(self.lastTrackInfo) > 0:
            track = self.lastTrackInfo[0]
            tPos = self.teamOrigin.relPtoB(track.pos())
            rel = tPos - pos
            enemy[0] = 1.0                                        # 探知有無
            enemy[1] = degrees(atan2(rel[1], rel[0]))             # 優先ターゲットの方位[deg]
            enemy[2] = float(np.linalg.norm(rel[:2])) / 1000.0    # 水平距離[km]
            enemy[3] = -tPos[2] / 1000.0                          # 高度[km]
            enemy[4] = len(self.lastTrackInfo)                    # 探知彼機数

        # --- 被弾警報(MWS) ---
        if parent.observables.contains_p("/sensor/mws/track"):
            for mObs in parent.observables.at_p("/sensor/mws/track"):
                mwsTrack = Track2D(mObs).transformTo(self.getLocalCRS())
                d = self.teamOrigin.relPtoB(mwsTrack.dir())
                threat[0] = 1.0
                threat[1] = degrees(atan2(d[1], d[0]))
                break

        return {"own": own, "enemy": enemy, "threat": threat}

    def observation_space(self):
        low = np.finfo(np.float32).min
        high = np.finfo(np.float32).max
        return spaces.Dict({  # type: ignore
            "own": spaces.Box(low, high, shape=[11], dtype=np.float32),
            "enemy": spaces.Box(low, high, shape=[5], dtype=np.float32),
            "threat": spaces.Box(low, high, shape=[2], dtype=np.float32),
        })

    def action_space(self):
        return spaces.Dict({  # type: ignore
            "turn": spaces.Discrete(len(self.turnTable)),
            "speed": spaces.Discrete(len(self.speedTable)),
            "target": spaces.Discrete(1 + self.maxEnemyNum),
        })

    def deploy(self, action):
        parent = self.parent
        if not parent.isAlive():
            return

        role_id = self.assign_role_id()
        my_motion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())
        pos = self.teamOrigin.relPtoB(my_motion.pos())

        az = radians(float(self.turnTable[int(action["turn"])]))
        dir_b = np.array([cos(az), sin(az), 0.0])
        self.dstDir = self.teamOrigin.relBtoP(dir_b)
        self.dstV = float(self.speedTable[int(action["speed"])])

        self.fired_log = 0
        self.launchFlag = False
        self.target = Track3D()
        tIdx = int(action["target"]) - 1
        if 0 <= tIdx < len(self.lastTrackInfo):
            candidate = self.lastTrackInfo[tIdx]
            if parent.isLaunchableAt(candidate) and self._countFlyingMissiles() < self.maxSimulShot:
                self.launchFlag = True
                self.target = candidate
                self.fired_log = 1

    def control(self):
        parent = self.parent
        if not parent.isAlive():
            return

        myMotion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())

        dstDir = np.array(self.dstDir, dtype=np.float64)
        n = sqrt(dstDir[0] * dstDir[0] + dstDir[1] * dstDir[1])
        if n < 1e-8:
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

        self.launchFlag = False

    def _countFlyingMissiles(self):
        count = 0
        if self.parent.observables.contains_p("/weapon/missiles"):
            for msl in self.parent.observables.at_p("/weapon/missiles"):
                if msl.at("isAlive")() and msl.at("hasLaunched")():
                    count += 1
        return count