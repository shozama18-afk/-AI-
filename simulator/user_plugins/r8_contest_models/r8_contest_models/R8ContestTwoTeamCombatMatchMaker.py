# Copyright (c) 2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
import os
import time
import datetime
import numpy as np
import gymnasium as gym
from collections import defaultdict
import copy
import cloudpickle
from ASRCAISim1.plugins.rayUtility.utility.TransparentRemoteObject import unwrap_if_needed
from ASRCAISim1.plugins.MatchMaker.TwoTeamCombatMatchMaker import TwoTeamCombatMatchMaker, getRandomizedInitialState, invert

def getForEscort(phase,p1,p2,radius,vel):
    #護衛対象機の初期位置を計算する
    dp=p2-p1
    distance=np.sqrt(dp[0]**2+dp[1]**2)
    ex=dp/np.linalg.norm(dp)
    ey=np.cross(np.array([0.,0.,1.]),ex)
    ey/=np.linalg.norm(ey)

    length=2*(radius*np.pi+distance)
    phase2rad=length/radius
    anchors=[
        (radius*np.pi/2.)/length,
        (radius*np.pi/2.+distance)/length,
        (radius*np.pi*3/2.+distance)/length,
        (radius*np.pi*3/2.+2*distance)/length,
    ]
    if phase < anchors[0]:
        angle=phase*phase2rad
        pos=p1+radius*(-ex*np.cos(angle)-ey*np.sin(angle))
        heading=90.0+angle/np.pi*180
    elif phase < anchors[1]:
        pos=p1+(ex*(phase-anchors[0])*length-ey*radius)
        heading=180.0
    elif phase < anchors[2]:
        angle=(phase-anchors[1])*phase2rad
        pos=p2+radius*(ex*np.sin(angle)-ey*np.cos(angle))
        heading=180.0+angle/np.pi*180
    elif phase < anchors[3]:
        pos=p2+(-ex*(phase-anchors[2])*length+ey*radius)
        heading=0.0
    else:
        angle=(1.0-phase)*phase2rad
        pos=p1+radius*(-ex*np.cos(angle)+ey*np.sin(angle))
        heading=90.0-angle/np.pi*180
    return {
        "pos":pos,
        "vel":vel,
        "heading": heading,
    }

def getFighterModelConfigs(division, table):
    # 各戦闘機の性能設定を行なう
    humanIntervention={
        "model":"r8_contest_models.R8ContestHumanIntervention",
        "config":{
            "delay": 3.0,
        }
    }
    fighter={
        "model":"r8_contest_models.R8Contest"+division+"Fighter",
        "config":{
            "pilot":{
                "model":"R8ContestHumanIntervention"
            },
            "sensor":{
                "radar":"R8Contest"+division+"AircraftRadar",
                "irst":"R8Contest"+division+"IRST"
            },
            "weapon":{
                "missile":"R8Contest"+division+"Missile",
                "numMsls":6
            },
            "stealth":{
                "rcsScale":1.0,
                "irRadiationIntensityScale":1.0,
            },
            "dynamics":{
                "scale":{
                    "vMin": 1.0,
                    "vMax": 1.0,
                }
            }
        }
    }
    missile={
        "model":"r8_contest_models.R8Contest"+division+"Missile",
        "config":{
            "thrust": 30017.9989,
        }
    }
    radar={
        "model":"r8_contest_models.R8Contest"+division+"AircraftRadar",
        "config":{
            "Lref": 60000.0 if division=="Youth" else 60000.0,
            "thetaFOR": 90.0 if division=="Youth" else 90.0
        }
    }
    irst={
        "model":"r8_contest_models.R8Contest"+division+"IRST",
        "config":{
            "Lref": -1.0 if division=="Youth" else 120000.0,
            "thetaFOR": -1.0 if division=="Youth" else 90.0
        }
    }

    if "radar_range" in table:
        radar["config"]["Lref"]*=np.random.uniform(table["radar_range"][0],table["radar_range"][1])
    if "radar_coverage" in table:
        radar["config"]["thetaFOR"]=np.random.uniform(table["radar_coverage"][0],table["radar_coverage"][1])
    if "irst_range" in table:
        irst["config"]["Lref"]*=np.random.uniform(table["irst_range"][0],table["irst_range"][1])
    if "irst_coverage" in table:
        irst["config"]["thetaFOR"]=np.random.uniform(table["irst_coverage"][0],table["irst_coverage"][1])
    if "rcs_scale" in table:
        fighter["config"]["stealth"]["rcsScale"]*=10.0**np.random.uniform(np.log10(table["rcs_scale"][0]),np.log10(table["rcs_scale"][1]))
    if "ir_radiation_intensity_scale" in table:
        fighter["config"]["stealth"]["irRadiationIntensityScale"]*=10.0**np.random.uniform(np.log10(table["ir_radiation_intensity_scale"][0]),np.log10(table["ir_radiation_intensity_scale"][1]))
    if "maximum_speed" in table:
        scale=np.random.uniform(table["maximum_speed"][0],table["maximum_speed"][1])
        fighter["config"]["dynamics"]["scale"]["vMax"]=scale
        if scale<1:
            fighter["config"]["dynamics"]["scale"]["vMin"]=scale
    if "num_missiles" in table:
        fighter["config"]["weapon"]["numMsls"]=np.random.randint(table["num_missiles"][0],table["num_missiles"][1])
    if "missile_thrust" in table:
        missile["config"]["thrust"]*=np.random.uniform(table["missile_thrust"][0],table["missile_thrust"][1])
    if "shot_approval_delay" in table:
        humanIntervention["config"]["delay"]=np.random.randint(table["shot_approval_delay"][0],table["shot_approval_delay"][1]+1)
    return humanIntervention, radar, irst, fighter, missile

def getEscortedModelConfigs(division, table):
    # 各護衛対象機の性能設定を行なう
    escorted={
        "model":"r8_contest_models.R8Contest"+division+"Escorted",
        "config":{
            "stealth":{
                "rcsScale":16.0,
                "irRadiationIntensityScale":4.0,
            },
            "dynamics":{
                "scale":{
                    "vMin": 1.0,
                    "vMax": 1.0,
                }
            },
        }
    }

    if "rcs_scale" in table:
        escorted["config"]["stealth"]["rcsScale"]*=10.0**np.random.uniform(np.log10(table["rcs_scale"][0]),np.log10(table["rcs_scale"][1]))
    if "ir_radiation_intensity_scale" in table:
        escorted["config"]["stealth"]["irRadiationIntensityScale"]*=10.0**np.random.uniform(np.log10(table["ir_radiation_intensity_scale"][0]),np.log10(table["ir_radiation_intensity_scale"][1]))
    if "maximum_speed" in table:
        scale_dummy1=np.random.uniform(table["maximum_speed"][0],table["maximum_speed"][1])
        scale_dummy2=np.random.uniform(table["maximum_speed"][0],table["maximum_speed"][1])
        if scale_dummy1 > scale_dummy2:
            escorted["config"]["dynamics"]["scale"]["vMax"]=scale_dummy1
            escorted["config"]["dynamics"]["scale"]["vMin"]=scale_dummy2
        else:
            escorted["config"]["dynamics"]["scale"]["vMax"]=scale_dummy2
            escorted["config"]["dynamics"]["scale"]["vMin"]=scale_dummy1
    return escorted

def getEscortedAgentModelConfigs(team, division, table):
    # 各護衛対象機用Agentの性能設定を行なう
    # 目標速度は"velocity"で指定する。実数値はそのまま、2要素リストはその値を上下限として一様サンプリング
    # 周回方向は"isClockwise"で指定する。"right"と"left"をキーとし選択確率を値とする辞書で指定する。確率の正規化は自動で行うので不要。
    escortedAgent={
        "model":"r8_contest_models.R8Contest"+team+"EscortedAgent",
        "config":{
            "velocity":250.0,
            "isClockwise":"right"
        }
    }
    if "velocity" in table:
        if isinstance(table["velocity"], float):
            # for backward compatibility
            escortedAgent["config"]["velocity"]=table["velocity"]
        else:
            escortedAgent["config"]["velocity"]=np.random.uniform(table["velocity"][0],table["velocity"][1])
        
    if "isClockwise" in table:
        if not isinstance(table["isClockwise"], dict):
            raise TypeError("'isClockwise' randomization parameter must be a dict.")
        if not ("right" in table["isClockwise"] and "left" in table["isClockwise"]):
            raise KeyError("'isClockwise' randomization parameter dict must contain 'right' and 'left' key.")
        escortedAgent["config"]["isClockwise"]=np.random.choice(["right","left"], p=[table["isClockwise"]["right"],table["isClockwise"]["left"]])
    return escortedAgent

class R8ContestTwoTeamCombatMatchMaker(TwoTeamCombatMatchMaker):
    def initialize(self,config):
        super().initialize(config)
        self.youth=self.match_config.get("youth",False)

    def makeNextMatch(self,matchType,worker_index):
        """対戦カードを生成する。Agent、Assetの初期配置、Assetの性能の3種類の設定を生成する。
        """
        #return super().makeNextMatch(matchType,worker_index)
        ret={
            "teams": {
                self.teams[0]:{},
                self.teams[1]:{},
            },
            "common": {
            }
        }
        ret["teams"]=self.setAgent(ret["teams"],matchType,worker_index)
        ret["teams"]=self.setInitialState(ret["teams"],matchType,worker_index)
        ret["common"]=self.setCommonCondition(ret["common"],matchType,worker_index)
        
        # 護衛対象機Agentの目標速度と周回方向を、初期配置にも反映する。
        # asset_slotでAgentとInitialStateを紐付けていた場合のみ反映可能。"Escorted1"として合わせておくこと。
        use_asset_slots = "asset_slots" in self.match_config
        if "initial_states" in self.match_config:
            if use_asset_slots:
                for config in self.match_config["initial_states"]:
                    if not "asset_slots" in config:
                        use_asset_slots = False
            if use_asset_slots:
                asset_slots = self.match_config["asset_slots"]

                for team in self.teams:
                    for sIdx, slot in enumerate(asset_slots):
                        if team+slot in ret["common"]["Factory"]["Agent"]:
                            # 同名slotのAgentが存在した場合のみ反映
                            vel = ret["common"]["Factory"]["Agent"][team+slot]["config"]["velocity"]
                            isClockwise = ret["common"]["Factory"]["Agent"][team+slot]["config"]["isClockwise"]

                            ret["teams"][team]["InitialState"][sIdx]["vel"] = vel
                            if isClockwise != "right":
                                ret["teams"][team]["InitialState"][sIdx]["heading"] += 180.0
        return ret

    def setAgent(self,ret,matchType,worker_index):
        """Agentの設定を行う。このサンプルでは親クラスと同様に、以下のように設定される。
        第一陣営(Blue)側：学習中の"Learner"又は一定確率(expert_ratio)で教師役の"Expert"
        第二陣営(Red)側：一定エピソード数(warm_up_episodes)の経過前は常に初期行動判断モデル("Initial")とし、
            経過後は(1)学習中重みの直近のコピー、(2)初期行動判断モデル("Initial")、(3)過去の保存された重みから一様分布で選択
            の3種類を4:4:2の割合で選択(ただし、(3)に該当する重みが存在しない場合は(1)とする。)
        expert_ratioとwarm_up_episodesはmatch_configで指定する。
        また、matchType,worker_indexは特に使用しない。
        """
        ret=super().setAgent(ret,matchType,worker_index)
        return ret

    def setInitialState(self,ret,matchType,worker_index):
        """各Assetの初期配置の設定を行う。
        このサンプルでは、pos,vel,headingの3種類の値を設定する。
        第一陣営(Blue)側の戦闘機は、match_configに"initial_state_lower"及び"initial_state_upper"として与えた上下限の範囲内でランダムに配置する。
        第二陣営(Red)側の戦闘機は、Blue側と点対称に配置する。
        護衛対象機は対称性を持たせず、周回軌道上のランダムな位相に配置する。
        """
        use_asset_slots = "asset_slot" in self.match_config

        if "initial_states" in self.match_config:
            if use_asset_slots:
                for config in self.match_config["initial_states"]:
                    if not "asset_slots" in config:
                        use_asset_slots = False
            if use_asset_slots:
                asset_slots = self.match_config["asset_slots"]

            for team in self.teams:
                if use_asset_slots:
                    ret[team]["InitialState"] = [{} for i in range(len(asset_slots))]
                else:
                    ret[team]["InitialState"]=[]

            for config in self.match_config["initial_states"]:
                sType = config.get("type","normal")
                if sType == "normal":
                    # 普通の機体
                    lower_bound=config["lower"]
                    upper_bound=config["upper"]
                    if self.youth:
                        lower_bound["pos"][2]=-10000.0
                        upper_bound["pos"][2]=-10000.0
                    initialStates=getRandomizedInitialState(
                        self.teams,
                        lower_bound,
                        upper_bound,
                        config.get("number",4),
                        config.get("symmetric",True)
                    )
                    for team in self.teams:
                        if use_asset_slots:
                            for slot, state in zip(config["asset_slots"], initialStates[team]["InitialState"]):
                                if slot in asset_slots:
                                    ret[team]["InitialState"][asset_slots.index(slot)] = state
                        else:
                            ret[team]["InitialState"].extend(initialStates[team]["InitialState"])
                elif sType == "escorted":
                    # 護衛対象機
                    p1=np.array(config.get("p1",[60000.0,100000.0,-10000.0]))
                    p2=np.array(config.get("p2",[-60000.0,100000.0,-10000.0]))
                    radius=config.get("radius",5000.0)
                    vel= config.get("vel",250.0)
                    for i in range(config.get("number",1)):
                        phase=np.random.rand()
                        if use_asset_slots:
                            slot = config["asset_slots"][i]
                            if slot in asset_slots:
                                ret[self.teams[0]]["InitialState"][asset_slots.index(slot)]=getForEscort(phase,p1,p2,radius,vel)
                        else:
                            ret[self.teams[0]]["InitialState"].append(getForEscort(phase,p1,p2,radius,vel))
                        if not config.get("symmetric",True):
                            phase=np.random.rand()
                        if use_asset_slots:
                            slot = config["asset_slots"][i]
                            if slot in asset_slots:
                                ret[self.teams[1]]["InitialState"][asset_slots.index(slot)]=invert(getForEscort(phase,p1,p2,radius,vel))
                        else:
                            ret[self.teams[1]]["InitialState"].append(invert(getForEscort(phase,p1,p2,radius,vel)))
                else:
                    raise KeyError("'type' of 'initial_states' must be one of ['normal','escorted']")

            return ret

    def setCommonCondition(self,ret,matchType,worker_index):
        """teamに依存しない条件(Rule等)を記述する。
        """
        ret=super().setCommonCondition(ret,matchType,worker_index)
        ret=self.setFactoryModel(ret,matchType,worker_index)
        return ret

    def setFactoryModel(self,ret,matchType,worker_index):
        """各Assetの性能に関する設定を行う。
        """

        division = "Youth" if self.youth else "Open"

        fac={
            "Controller":{
            },
            "PhysicalAsset":{
            },
            "Agent":{
            }
        }

        if "asset_spec_randomization" in self.match_config:
            asset_spec_randomization = self.match_config["asset_spec_randomization"]
            for assetGroup, randomization_config in asset_spec_randomization.items():
                number=randomization_config["number"]
                assetType=randomization_config["type"]
                symmetric=randomization_config["symmetric"]
                heterogeneous=randomization_config["heterogeneous"]
                shuffle=randomization_config["shuffle"]
                table=randomization_config.get("randomized_asset_spec",{})
                slots=randomization_config.get("asset_slots", [assetType+str(i+1) for i in range(number)])
                if table is None:
                    table={}
                if assetType == "Fighter":
                    order = np.random.permutation(range(number)) if shuffle else np.array(range(number))
                    humanIntervention, radar, irst, fighter, missile = None, None, None, None, None
                    for i in range(number):
                        if heterogeneous or humanIntervention is None:
                            humanIntervention, radar, irst, fighter, missile = getFighterModelConfigs(division,table)
                        for tIdx, team in enumerate(self.teams):
                            if tIdx>0 and not symmetric:
                                humanIntervention, radar, irst, fighter, missile = getFighterModelConfigs(division,table)
                            index = i if tIdx==0 else order[i]
                            fac["PhysicalAsset"][team+slots[index]]=copy.deepcopy(fighter)

                            fac["Controller"][team+slots[index]+"HumanIntervention"]=copy.deepcopy(humanIntervention)
                            fac["PhysicalAsset"][team+slots[index]+"AircraftRadar"]=copy.deepcopy(radar)
                            fac["PhysicalAsset"][team+slots[index]+"IRST"]=copy.deepcopy(irst)
                            fac["PhysicalAsset"][team+slots[index]+"Missile"]=copy.deepcopy(missile)

                    for i in range(number):
                        for tIdx, team in enumerate(self.teams):
                            fac["PhysicalAsset"][team+slots[i]]["config"]["pilot"]["model"]=team+slots[i]+"HumanIntervention"
                            fac["PhysicalAsset"][team+slots[i]]["config"]["weapon"]["missile"]=team+slots[i]+"Missile"
                            fac["PhysicalAsset"][team+slots[i]]["config"]["sensor"]["radar"]=team+slots[i]+"AircraftRadar"
                            fac["PhysicalAsset"][team+slots[i]]["config"]["sensor"]["irst"]=team+slots[i]+"IRST"
                elif assetType == "Escorted":
                    order = np.random.permutation(range(number)) if shuffle else np.array(range(number))
                    escorted = None
                    for i in range(number):
                        if heterogeneous or escorted is None:
                            escorted = getEscortedModelConfigs(division,table)
                        for tIdx, team in enumerate(self.teams):
                            if tIdx>0 and not symmetric:
                                escorted = getEscortedModelConfigs(division,table)
                            index = i if tIdx==0 else order[i]
                            fac["PhysicalAsset"][team+slots[index]]=copy.deepcopy(escorted)
                elif assetType == "EscortedAgent":
                    order = np.random.permutation(range(number)) if shuffle else np.array(range(number))
                    escortedAgent = None
                    for i in range(number):
                        for tIdx, team in enumerate(self.teams):
                            if tIdx==0:
                                if heterogeneous or escortedAgent is None:
                                    escortedAgent = getEscortedAgentModelConfigs(team,division,table)
                            elif not symmetric:
                                escortedAgent = getEscortedAgentModelConfigs(team,division,table)
                            index = i if tIdx==0 else order[i]
                            fac["Agent"][team+slots[index]]=copy.deepcopy(escortedAgent)
                else:
                    raise KeyError(f"{assetType} is not registered as an Asset type for spec randomization in {type(self)}.")
        ret["Factory"]=fac
        return ret
