# Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
import os
import sys
import json
import time
import argparse
import importlib
import copy
from collections import defaultdict
import gymnasium as gym
import numpy as np
from ASRCAISim1.core import Factory, nljson, Fighter, Entity
from ASRCAISim1.common import addPythonClass
from ASRCAISim1.GymManager import GymManager
from ASRCAISim1.plugins.rayUtility.utility.TransparentRemoteObject import unwrap_if_needed
from ASRCAISim1.plugins.MatchMaker.TwoTeamCombatMatchMaker import (
    getFromDesiredInitialState,
    getRandomizedInitialState,
    invert,
)

from r8_contest_models.R8ContestTwoTeamCombatMatchMaker import (
    getForEscort,
    R8ContestTwoTeamCombatMatchMaker
)

def managerConfigReplacer(config,matchInfo):
    """対戦カードを表すmatchInfoに従い、SimulationManagerのコンフィグを置き換える関数。
    configはSimulationManager::getManagerConfig()で得られるものであり、
    Simulationmanagerのコンストラクタに渡すenv_configのうち"Manager"キー以下の部分となる。
    """
    ret=copy.deepcopy(config)
    agentConfigDispatcher=ret["AgentConfigDispatcher"]
    assetConfigDispatcher=ret["AssetConfigDispatcher"]
    for team, info in matchInfo.get("teams",matchInfo).items():
        if team == "Factory":
            continue
        agentConfigDispatcher[team+"Agents"]=info["Agent"]
        if "InitialState" in info:
            num=len(assetConfigDispatcher[team+"InitialState"]["elements"])
            for i in range(num):
                assetConfigDispatcher[team+"InitialState"]["elements"][i]["value"]["instanceConfig"]=info["InitialState"][i]
    return ret
def factoryConfigReplacer(config,matchInfo):
    """対戦カードを表すmatchInfoに従い、SimulationManagerのFactory側のコンフィグを置き換える関数。
    configはSimulationManager::getFactoryModelConfig(True)で得られるものであり、
    Simulationmanagerのコンストラクタに渡すenv_configのうち"Factory"キー以下の部分となる。
    """
    ret=nljson(config)
    if "Factory" in matchInfo.get("common", {}):
        ret.merge_patch(matchInfo["common"]["Factory"])

    return ret

def wrapEnvForTwoTeamCombatMatchMaking(base_class):
    """MatchMakerの付帯情報をgym.Envに付加するラッパー
    """
    class WrappedEnvForTwoTeamCombatMatchMaking(base_class):
        def setMatch(self,matchInfo):
            self.matchInfo=matchInfo
            self.requestReconfigure(
                managerConfigReplacer(unwrap_if_needed(self.manager.getManagerConfig()()),matchInfo),
                factoryConfigReplacer(unwrap_if_needed(self.manager.getFactoryModelConfig(True)()),matchInfo)
            )
    return WrappedEnvForTwoTeamCombatMatchMaking

class R8ContestMatchMakerForEvaluation(R8ContestTwoTeamCombatMatchMaker):
    # R8年度コンテストの評価サーバ用のMatchMaker
    def initialize(self,config):
        super().initialize(config)
        self.teams = list(self.match_config["teams"].keys())
        self.team_config=self.match_config["teams"]
        self.number=4
    def setAgent(self,ret,matchType,worker_index):
        for team in self.teams:
            userModelID=self.team_config[team]["userModelID"]
            isSingle=self.team_config[team]["isSingle"]
            if(isSingle):
                # 1個のインスタンスで1機なので異なるnameを割り当て、portはどちらも"0"
                ret[team]={"Agent":{
                    "type": "group",
                    "order": "fixed",
                    "elements": [
                        {"type": "External", "model": "Agent_"+userModelID, "policy": "Policy_" +
                            userModelID, "name": team+"_"+userModelID+"_"+str(i+1), "port": "0"} for i in range(self.number)
                    ]
                }}
            else:
                # 1個のインスタンスで2機分なので同じnameを割り当てることで1個のインスタンスとし、それぞれ異なるport("0"からstr(self.number-1))を割り当てる
                ret[team]={"Agent":{
                    "type": "group",
                    "order": "fixed",
                    "elements": [
                        {"type": "External", "model": "Agent_"+userModelID,
                            "policy": "Policy_"+userModelID, "name": team+"_"+userModelID, "port": str(i)} for i in range(self.number)
                    ]
                }}
        return ret

def run(match_config):
    seed=match_config.get("seed",None)
    if(seed is None):
        import numpy as np
        seed = np.random.randint(2**31)
    logName="_vs_".join([info.get("logName",info["userModelID"]) for info in match_config["teams"].values()])
    # ユーザーモジュールの読み込み
    importedUserModules={}
    for team,info in match_config["teams"].items():
        userModelID=info["userModelID"]
        userModuleID=info["userModuleID"]
        args=info.get("args",{})
        args["userModelID"]=userModelID
        info["args"]=args
        if(not userModuleID in importedUserModules):
            try:
                module = importlib.import_module(userModuleID)
                assert hasattr(module, "getUserAgentClass")
                assert hasattr(module, "getUserAgentModelConfig")
                assert hasattr(module, "isUserAgentSingleAsset")
                assert hasattr(module, "getUserPolicy")
            except Exception as e:
                raise e  # 読み込み失敗時の扱いは要検討
            importedUserModules[userModuleID]=module
        module=importedUserModules[userModuleID]
        agentClass = module.getUserAgentClass(args)
        addPythonClass("Agent", "Agent_"+userModelID, agentClass)
        Factory.addDefaultModel(
            "Agent",
            "Agent_"+userModelID,
            {
                "class": "Agent_"+userModelID,
                "config": module.getUserAgentModelConfig(args)
            }
        )
    # コンフィグの生成
    loggerConfig = {}
    if "log_dir" in match_config:
        loggerConfig = {
            "MultiEpisodeLogger":{
                "class":"MultiEpisodeLogger",
                "config":{
                    "prefix":os.path.join(match_config["log_dir"],logName),
                    "episodeInterval":1,
                    "ratingDenominator":100
                }
            }
        }
        if match_config["replay"]:
            loggerConfig["GUIStateLogger"]={
                "class":"GUIStateLogger",
                "config":{
                    "prefix":os.path.join(match_config["log_dir"],logName),
                    "episodeInterval":1,
                    "innerInterval":1
                }
            }

    division="youth" if match_config["youth"] else "open"
    configs = [
        os.path.join(os.path.dirname(__file__), "common/R8_contest_"+division+"_mission_config.json"),
        os.path.join(os.path.dirname(__file__), "common/R8_contest_"+division+"_asset_placeholder.json"),
        {
            "Manager": {
                "Rewards": [],
                "seed":seed,
                "ViewerType":"R8Contest"+("Youth" if match_config["youth"] else "Open")+"God" if match_config["visualize"] else "None",
                "Loggers": loggerConfig,
                "AgentConfigDispatcher": {}
            }
        }
    ]
    context = {
        "config": configs,
        "worker_index": 0,
        "vector_index": 0
    }
    # 環境の生成
    env = wrapEnvForTwoTeamCombatMatchMaking(GymManager)(context)

    # MatchMakerの生成
    for team,info in match_config["teams"].items():
        info["isSingle"]=importedUserModules[info["userModuleID"]].isUserAgentSingleAsset(info["args"])

    match_config["asset_slots"] = ["Fighter1", "Fighter2", "Fighter3", "Fighter4", "Escorted1"]
    match_config["initial_states"] = [
        {
            "number": 4,
            "type": "normal",
            "asset_slots": ["Fighter1", "Fighter2", "Fighter3", "Fighter4"],
            "symmetric": True,
            "lower": {
                "pos": [-20000.0,95000.0,-12000.],
                "vel": 260.0,
                "heading": 225.0
            },
            "upper": {
                "pos": [20000.0,105000.0,-6000.0],
                "vel": 280.0,
                "heading": 315.0
            }
        },
        {
            "number": 1,
            "type": "escorted",
            "asset_slots": ["Escorted1"],
            "symmetric": False,
            "p1": [100000.0,120000.0,-10000.0],
            "p2": [-100000.0,120000.0,-10000.0],
            "radius": 5000.0,
            "vel": 250.0 # 護衛対象機Agentの目標速度で上書きされるので指定無効
        }
    ]
    if not match_config["youth"]:
        # オープン部門は性能をランダム化
        match_config["asset_spec_randomization"] = {
            "fighter": {
                "number": 4,
                "type": "Fighter",
                "asset_slots": ["Fighter1", "Fighter2", "Fighter3", "Fighter4"],
                "symmetric": True,
                "heterogeneous": True,
                "shuffle": True,
                "randomized_asset_spec": {
                    # ランダムにしない値はコメントアウトすること。
                    "rcs_scale": [0.4,1.0], # 探知距離にして約0.8〜1.0倍
                    "radar_range": [0.8,1.2],
                    "radar_coverage": [60.0,120.0],
                    "ir_radiation_intensity_scale": [0.64,1.0], # 探知距離にして約0.8〜1.0倍
                    "irst_range": [0.8,1.2],
                    "irst_coverage": [60.0,120.0],
                    "maximum_speed": [0.8,1.2],
                    "num_missiles": [4,8],
                    "missile_thrust": [0.8,1.2],
                    "shot_approval_delay": [1,5]
                }
            },
            "escorted": {
                "number": 1,
                "type": "Escorted",
                "asset_slots": ["Escorted1"],
                "symmetric": True,
                "heterogeneous": True,
                "shuffle": True,
                "randomized_asset_spec": {
                    #"rcs_scale": [0.4,1.0], # 探知距離にして約1.6〜2.0倍
                    #"ir_radiation_intensity_scale": [0.64,1.0], # 探知距離にして約1.6〜2.0倍
                    #"maximum_speed": [0.9,1.1], 
                }
            },
            "escorted_agent": {
                "number": 1,
                "type": "EscortedAgent",
                "asset_slots": ["Escorted1"], # escortedのasset_slotsと一致させること。
                "symmetric": False,
                "heterogeneous": True,
                "shuffle": True,
                "randomized_asset_spec": {
                    "velocity": [225.0, 275.0], #目標速度 = 初期速度として設定
                    "isClockwise": { # 周回方向をそれぞれの確率で指定
                        "right": 0.5,
                        "left": 0.5
                    }
                }
            }
        }

    matchMaker = R8ContestMatchMakerForEvaluation({"match_config":match_config})

    # StandalonePolicyの生成
    policies = {
        "Policy_"+info["userModelID"]: importedUserModules[info["userModuleID"]].getUserPolicy(info["args"])
        for team,info in match_config["teams"].items()
    }
    # policyMapperの定義(基本はデフォルト通り)

    def policyMapper(fullName):
        agentName, modelName, policyName = fullName.split(":")
        return policyName

    # 生成状況の確認
    matchInfo=matchMaker.makeNextMatch("",0)
    env.setMatch(matchInfo)
    obs, info = env.reset() # 確認前にresetでエピソードを初期化しておく必要あり
    observation_space = env.observation_space
    action_space = env.action_space
    print("=====Agent classes=====")
    for team,info in match_config["teams"].items():
        print("Agent_"+info["userModelID"], " = ", importedUserModules[info["userModuleID"]].getUserAgentClass(info["args"]))
    print("=====Policies=====")
    for name, policy in policies.items():
        print(name, " = ", type(policy))
    print("=====Policy Map (at reset)=====")
    for fullName in action_space:
        print(fullName, " -> ", policyMapper(fullName))
    print("=====Agent to Asset map=====")
    for agent in [a() for a in env.manager.getAgents()]:
        print(agent.getFullName(), " -> ", "{")
        for port, parent in agent.parents.items():
            print("  ", port, " : ", parent.getFullName())
        print("}")

    # 対戦スコア計算用の変数
    teams = [team for team in match_config["teams"].keys()]
    userModelIDs = {
        team: teamInfo["userModelID"]
        for team, teamInfo in match_config["teams"].items()
    }
    opponents = {
        userModelIDs[teams[0]]: userModelIDs[teams[1]],
        userModelIDs[teams[1]]: userModelIDs[teams[0]],
    }
    totalScores = defaultdict(lambda: 0)
    # 各戦闘（エピソード）の勝利時に、以下の要領で戦闘スコアを計算する。
    # 所定のエピソード数が終了した時点の戦闘スコアの合計値を対戦スコアとし、対戦スコアの大きい方を対戦の勝者として扱う。
    # 
    # - 失格による決着の場合、失格となっていない陣営を勝者としてに護衛対象機撃破時と同じ得点計算を行う。双方失格の場合は引き分けとして+0.5点。
    # - 護衛対象機の撃破（撃墜又は相手の自滅）による決着の場合、先に撃破した陣営を勝者として0.6 + 0.3 * (命中弾数/射撃弾数) + 0.1 * min(1, max(0, 2-経過秒数/900))点を与え、敗者の得点は1-(勝者の得点)とする。同時撃破の場合は引き分けとして双方に+0.5点。

    # シミュレーションの実行
    print("=====running simulation(s)=====")
    numEpisodes = match_config["num_episodes"]

    for episodeCount in range(numEpisodes):
        start=time.time()
        if episodeCount > 0 and episodeCount % 2 == 0:
            # 偶数回目は初期配置や機体性能も再生成
            matchInfo=matchMaker.makeNextMatch("",0)
            env.setMatch(matchInfo)
        elif episodeCount % 2 == 1:
            # 奇数回目は行動判断モデルだけ入れ替えてもう一回
            tmp=matchInfo["teams"][teams[0]]["Agent"]
            matchInfo["teams"][teams[0]]["Agent"]=matchInfo["teams"][teams[1]]["Agent"]
            matchInfo["teams"][teams[1]]["Agent"]=tmp

            env.setMatch(matchInfo)
        if episodeCount > 0:
            # 初回は生成状況の確認時にreset済
            obs, info = env.reset()

        rewards = {k: 0.0 for k in obs.keys()}
        terminateds = {k: False for k in obs.keys()}
        truncateds = {k: False for k in obs.keys()}
        infos = {k: None for k in obs.keys()}
        for p in policies.values():
            p.reset()

        # 各陣営の初期弾数の計算
        initialNumMissiles = defaultdict(lambda: 0)
        #initialNumAircrafts = defaultdict(lambda: 0)
        for team, teamInfo in matchInfo["teams"].items():
            userModelID = userModelIDs[team]
            for wFighter in env.manager.getAssets(lambda a:a.getTeam()==team and isinstance(a,Fighter)):
                # 戦闘機も護衛対象機もFighterクラス
                fighter = wFighter() # weakref(weak_ptr)から強参照に変更
                initialNumMissiles[userModelID] += fighter.numMsls
                #initialNumAircrafts[userModelID] += 1

        # 各陣営の命中数のカウント
        hitCounts = defaultdict(lambda: 0)

        def onHit(args: nljson):
            # 命中数を数えるためのイベントハンドラ
            # argsは{"wpn":wpn,"tgt":tgt}
            wpn = Entity.from_json(args["wpn"])
            #tgt = Entity.from_json(args["tgt"])
            hitCounts[userModelIDs[wpn.getTeam()]] += 1

        # イベントハンドラはSimulationManagerAccessorForCallbackからしか追加できないのでRuler経由で取得して追加する。
        managerAccessorForCB=env.manager.getRuler()().manager
        managerAccessorForCB.addEventHandler("Hit",onHit)

        terminateds["__all__"]=False
        truncateds["__all__"]=False
        while not (terminateds["__all__"] or truncateds["__all__"]):
            observation_space = env.get_observation_space()
            action_space = env.get_action_space()
            actions = {k: policies[policyMapper(k)].step(
                o,
                rewards[k],
                terminateds[k] or truncateds[k],
                infos[k],
                k,
                observation_space[k],
                action_space[k]
            ) for k, o in obs.items() if policyMapper(k) in policies}
            obs, rewards, terminateds, truncateds, infos = env.step(actions)
        took=time.time()-start

        # 戦闘スコアを計算
        remainedNumMissiles = defaultdict(lambda: 0)
        #survivedNumAircrafts =defaultdict(lambda: 0)
        #wastedMissiles = defaultdict(lambda: 0) # 非命中弾数
        launchedMissiles = defaultdict(lambda: 0) # 射撃弾数
        episodeScores =defaultdict(lambda: 0)
        for team, teamInfo in matchInfo["teams"].items():
            for wFighter in env.manager.getAssets(lambda a:a.getTeam()==team and isinstance(a,Fighter)):
                # 戦闘機も護衛対象機もFighterクラス
                fighter = wFighter() # weakref(weak_ptr)から強参照に変更
                userModelID = userModelIDs[team]
                remainedNumMissiles[userModelID] += fighter.remMsls
                #if fighter.isAlive():
                #    survivedNumAircrafts[userModelID] += 1

        for team, teamInfo in matchInfo["teams"].items():
            userModelID = userModelIDs[team]
            launchedMissiles[userModelID] = initialNumMissiles[userModelID] - remainedNumMissiles[userModelID]
            #hitCount = initialNumAircrafts[opponents[userModelID]] - survivedNumAircrafts[opponents[userModelID]]
            #wastedMissiles[userModelID] = launchedMissiles[userModelID] - hitCount

        ruler = env.manager.getRuler()()
        endReason = ruler.getEndReason()
        spentTime = env.manager.getElapsedTime()
        winnerUserModelID = userModelIDs.get(env.manager.getRuler()().winner, "Draw")
        def envManagerGetRulerloser(winnerUserModelID):
            if winnerUserModelID == "Red":
                loser = "Blue"
            elif winnerUserModelID == "Blue":
                loser = "Red"
            else:
                loser ="Draw"
            loserUserModelID = userModelIDs.get(loser, "Draw")
            return loserUserModelID
        loserUserModelID = envManagerGetRulerloser(winnerUserModelID)

        if endReason == "PENALTY":
            # 失格で決着した場合
            if winnerUserModelID == "Draw":
                # 双方失格の場合は引き分けなのでどちらも+0.5点
                for team, teamInfo in matchInfo["teams"].items():
                    userModelID = userModelIDs[team]
                    episodeScores[userModelID] += 0.5
            else:
                # 一方だけ失格の場合は、失格になっていない方に護衛対象機撃破時と同等の対戦スコアを加算し、失格者には（1-勝者の得点）を加算
                winnerScore = (
                    0.6
                    + 0.3 * (hitCounts[winnerUserModelID] / launchedMissiles[winnerUserModelID] if launchedMissiles[winnerUserModelID] > 0 else 0.0)
                    + 0.1 * (1 if spentTime < 900.0 else max(0, 2.0-spentTime/900.0))
                )
                episodeScores[winnerUserModelID] += winnerScore
                episodeScores[loserUserModelID] += 1.0 - winnerScore
        elif endReason == "ELIMINATION":
            # 護衛対象機の撃破で決着した場合
            if winnerUserModelID == "Draw":
                # 引き分けの場合は双方に+0.5点加算
                for team, teamInfo in matchInfo["teams"].items():
                    userModelID = userModelIDs[team]
                    episodeScores[userModelID] += 0.5
            else:
                # そうでない場合は、勝者に命中率と撃墜までの速さを加味した得点を加算し、敗者には（1-勝者の得点）を加算
                winnerScore = (
                    0.6
                    + 0.3 * (hitCounts[winnerUserModelID] / launchedMissiles[winnerUserModelID] if launchedMissiles[winnerUserModelID] > 0 else 0.0)
                    + 0.1 * (1 if spentTime < 900.0 else max(0, 2.0-spentTime/900.0))
                )
                episodeScores[winnerUserModelID] += winnerScore
                episodeScores[loserUserModelID] += 1.0 - winnerScore
        else:
            # 時間切れの場合は引き分けなのでどちらも+0.5点
            for team, teamInfo in matchInfo["teams"].items():
                userModelID = userModelIDs[team]
                episodeScores[userModelID] += 0.5
            pass

        for team, teamInfo in matchInfo["teams"].items():
            userModelID = userModelIDs[team]

        for team, teamInfo in matchInfo["teams"].items():
            userModelID = userModelIDs[team]
            totalScores[userModelID] += episodeScores[userModelID]

        print(f"episode({episodeCount+1}/{numEpisodes})"
             ,f"winner={winnerUserModelID}"
             ,f"endReason={endReason}"
             ,f"scores={dict(episodeScores)}"
             ,f"calc. time={took} sec, sim. time={spentTime} sec,"
             ,f"hitCounts={dict(hitCounts)}"
             ,f"launchedMissiles={dict(launchedMissiles)}"
        )

    print("=====Total Result=====")
    if totalScores[userModelIDs[teams[0]]] > totalScores[userModelIDs[teams[1]]]:
        winner = userModelIDs[teams[0]]
    elif totalScores[userModelIDs[teams[1]]] > totalScores[userModelIDs[teams[0]]]:
        winner = userModelIDs[teams[1]]
    else:
        winner = "Draw"
    print(f"winner={winner}"
         ,f"\ntotalScores={dict(totalScores)}"
    )
    return winner, totalScores

if __name__ == "__main__":
    candidates=json.load(open("candidates.json","r"))
    parser=argparse.ArgumentParser()
    parser.add_argument("Blue",type=str,help="name of Blue team")
    parser.add_argument("Red",type=str,help="name of Red team")
    parser.add_argument("-n","--num_episodes",type=int,default=1,help="number of evaluation episodes")
    parser.add_argument("-l","--log_dir",type=str,help="log directory")
    parser.add_argument("-r", "--replay",action="store_true",help="use when you want to record episodes for later visualization")
    parser.add_argument("-v","--visualize",action="store_true",help="use when you want to visualize episodes")
    parser.add_argument("-y","--youth",action="store_true",help="use when you want to evaluate models for youth-division")
    args=parser.parse_args()
    assert(args.Blue in candidates and args.Red in candidates)
    match_config={
        "teams":{
            "Blue":{"userModelID":args.Blue},
            "Red":{"userModelID":args.Red},
        },
        "num_episodes":args.num_episodes,
        "replay":args.replay,
        "visualize":args.visualize,
        "youth":args.youth,
    }
    for team,value in match_config["teams"].items():
        match_config["teams"][team].update(candidates[value["userModelID"]])

    # 同一モデルを指定した場合は区別する接尾辞を付加する。
    if args.Blue == args.Red:
        match_config["teams"]["Blue"]["userModelID"] = match_config["teams"]["Blue"]["userModelID"] + "_Alpha"
        match_config["teams"]["Red"]["userModelID"] = match_config["teams"]["Red"]["userModelID"] + "_Bravo"

    if(args.log_dir is not None):
        match_config["log_dir"]=args.log_dir
    run(match_config)
