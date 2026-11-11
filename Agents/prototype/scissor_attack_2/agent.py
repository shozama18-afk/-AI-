"""挟み撃ち・VIP強襲 ＆ 速度・進行角観測対応 Agent (S方式)"""

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
    """観測情報の取得・VIP抽出・生存機数・敵進行方位判定機能を備えた Agent クラス"""

    def assign_role_id(self):
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
                fighter_positions.append((fullName, pos[1]))

        if len(fighter_positions) == 0:
            return None

        fighter_positions.sort(key=lambda x: x[1], reverse=True)

        my_name = parent.getFullName()
        for rank, (name, _) in enumerate(fighter_positions, start=1):
            if name == my_name:
                self.my_role_id = rank
                break

        return self.my_role_id

    def initialize(self):
        super().initialize()
        self.own = self.getTeam()
        self.my_role_id = None
        mc = self.modelConfig

        self.turnTable = np.array(
            getValueFromJsonKRD(mc, "turnTable", self.randomGen,
                                [-180, -170.0, -160.0, -150.0, -140.0, -130.0, -120.0, -110.0, -100.0, -90.0, -80.0, -70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0, 180.0]),
            dtype=np.float64)
        self.speedTable = np.array(
            getValueFromJsonKRD(mc, "speedTable", self.randomGen, [400.0, 420.0, 440.0, 460.0, 480.0, 500.0, 520.0, 540.0, 560.0]),
            dtype=np.float64)
        self.maxEnemyNum = int(getValueFromJsonKRD(mc, "maxEnemyNum", self.randomGen, 5))

        self.altMin = getValueFromJsonKRD(mc, "altMin", self.randomGen, 2000.0)
        self.altMax = getValueFromJsonKRD(mc, "altMax", self.randomGen, 15000.0)
        self.maxSimulShot = int(getValueFromJsonKRD(mc, "maxSimulShot", self.randomGen, 2))
        self.altitudeKeeper = AltitudeKeeper(mc().get("altitudeKeeper", {}))

        self.lastTrackInfo = []
        self.dstDir = np.array([1.0, 0.0, 0.0])
        self.dstV = float(self.speedTable[-1])
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

        own = np.zeros([15], dtype=np.float32)   # 15次元 [0~7:運動/余白, 8:発射, 9:Role, 10:残弾, 11:時間, 12:自機数, 13:敵機数, 14:迂回部隊平均X]
        enemy = np.zeros([7], dtype=np.float32) # 7次元 [0:探知, 1:方位, 2:距離, 3:高度, 4:探知数, 5:Vc, 6:敵進行角自機指向フラグ]
        threat = np.zeros([2], dtype=np.float32)
        vip = np.zeros([6], dtype=np.float32)
        self.lastTrackInfo = []

        if not parent.isAlive():
            return {"own": own, "enemy": enemy, "threat": threat, "vip": vip}

        # 自機運動状態
        myMotion = MotionState(parent.observables["motion"]).transformTo(self.getLocalCRS())
        pos = self.teamOrigin.relPtoB(myMotion.pos())
        vel = self.teamOrigin.relPtoB(myMotion.vel())
        V = float(np.linalg.norm(vel))

        own[0] = pos[0] / 1000.0
        own[1] = pos[1] / 1000.0
        own[2] = -pos[2] / 1000.0
        own[3] = V
        own[4] = degrees(atan2(vel[1], vel[0])) if V > 1e-5 else 0.0
        own[5] = (self.dOut - abs(pos[1])) / 1000.0
        own[6] = (self.dLine - pos[0]) / 1000.0
        own[7] = (self.dLine + pos[0]) / 1000.0
        own[8] = float(self.fired_log)

        ally_tag = self.assign_role_id()
        own[9] = float(ally_tag if ally_tag is not None else 1)

        if parent.observables.contains_p("/weapon/remMsls"):
            own[10] = float(parent.observables.at_p("/weapon/remMsls"))

        # --- 経過時間 ＆ 生存機数 ＆ 迂回部隊平均X座標 ---
        own[11] = float(self.manager.getElapsedTime())

        num_alive_allies = 0
        flank_x_list = []
        if parent.observables.contains_p("/shared/fighter"):
            shared_fighters = parent.observables.at_p("/shared/fighter")
            for fullName, fObs in shared_fighters.items():
                is_alive = False
                if hasattr(fObs, "contains_p") and fObs.contains_p("isAlive"):
                    is_alive = bool(fObs.at("isAlive")())
                elif isinstance(fObs, dict):
                    is_alive = bool(fObs.get("isAlive", False))
                if is_alive:
                    num_alive_allies += 1
                    # 味方のX座標取得
                    if hasattr(fObs, "contains_p") and fObs.contains_p("motion"):
                        m = MotionState(fObs["motion"]).transformTo(self.getLocalCRS())
                        p = self.teamOrigin.relPtoB(m.pos())
                        # Y座標の絶対値が大きい外側2機を迂回部隊と判定
                        if abs(p[1]) > 10000.0:  # 横方向に10km以上離れている外側機
                            flank_x_list.append(p[0] / 1000.0)

        if num_alive_allies == 0 and parent.isAlive():
            num_alive_allies = 1
        own[12] = float(num_alive_allies)
        own[14] = float(np.mean(flank_x_list)) if len(flank_x_list) > 0 else own[0]

        # 彼機航跡
        self.lastTrackInfo = [
            Track3D(t).transformTo(self.getLocalCRS())
            for t in parent.observables.at_p("/sensor/track")
        ]
        sortTrack3DByDistance(self.lastTrackInfo, [myMotion], True)
        own[13] = float(len(self.lastTrackInfo))

        # --- 最寄りの彼機 ＆ Vc ＆ 進行ベクトル判定 ---
        if len(self.lastTrackInfo) > 0:
            track = self.lastTrackInfo[0]
            tPos = self.teamOrigin.relPtoB(track.pos())
            rel = tPos - pos
            rel_pos_2d = rel[:2]
            dist_2d = np.linalg.norm(rel_pos_2d)

            Vc = 0.0
            is_enemy_heading_to_me = 0.0

            if dist_2d > 1e-5:
                los_unit = rel_pos_2d / dist_2d  # 自機 -> 敵機 単位ベクトル
                my_v_2d = vel[:2]
                t_vel = self.teamOrigin.relPtoB(track.vel())
                t_v_2d = t_vel[:2]
                rel_v_2d = my_v_2d - t_v_2d
                Vc = float(np.dot(rel_v_2d, los_unit))

                # 敵の進行方向ベクトルが自機（-los_unit）を向いているか判定（角度差30°以内 -> cos >= 0.866）
                t_v_norm = np.linalg.norm(t_v_2d)
                if t_v_norm > 1.0:
                    t_v_dir = t_v_2d / t_v_norm
                    vec_to_me = -los_unit  # 敵から自機への単位ベクトル
                    cos_sim = np.dot(t_v_dir, vec_to_me)
                    if cos_sim >= 0.866:  # cos(30 deg) ≈ 0.866
                        is_enemy_heading_to_me = 1.0

            enemy[0] = 1.0
            enemy[1] = degrees(atan2(rel[1], rel[0]))
            enemy[2] = float(np.linalg.norm(rel[:2])) / 1000.0
            enemy[3] = -tPos[2] / 1000.0
            enemy[4] = float(len(self.lastTrackInfo))
            enemy[5] = Vc
            enemy[6] = is_enemy_heading_to_me  # 敵進行角30°以内自機指向フラグ

        # VIP抽出
        vip_track = None
        for track in self.lastTrackInfo:
            t_json = track.to_json()
            if t_json.get("label", 0) == 1:
                vip_track = track
                break

        if vip_track is not None:
            vPos = self.teamOrigin.relPtoB(vip_track.pos())
            vRel = vPos - pos
            vip[0] = 1.0
            vip[1] = degrees(atan2(vRel[1], vRel[0]))
            vip[2] = float(np.linalg.norm(vRel[:2])) / 1000.0
            vip[3] = -vPos[2] / 1000.0
            vip[4] = vPos[1] / 1000.0
            vip[5] = 1.0

        # 被弾警報 (MWS)
        if parent.observables.contains_p("/sensor/mws/track"):
            for mObs in parent.observables.at_p("/sensor/mws/track"):
                mwsTrack = Track2D(mObs).transformTo(self.getLocalCRS())
                d = self.teamOrigin.relPtoB(mwsTrack.dir())
                threat[0] = 1.0
                threat[1] = degrees(atan2(d[1], d[0]))
                break

        return {"own": own, "enemy": enemy, "threat": threat, "vip": vip}

    def observation_space(self):
        low = np.finfo(np.float32).min
        high = np.finfo(np.float32).max
        return spaces.Dict({  # type: ignore
            "own": spaces.Box(low, high, shape=[15], dtype=np.float32),
            "enemy": spaces.Box(low, high, shape=[7], dtype=np.float32),
            "threat": spaces.Box(low, high, shape=[2], dtype=np.float32),
            "vip": spaces.Box(low, high, shape=[6], dtype=np.float32),
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