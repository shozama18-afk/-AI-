
"""!
Copyright (c) 2026-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
@package BasicAgentUtility.util.imitation.convertActionFromAnotherAgent

@brief 模倣学習用の行動空間変換を行うためのユーティリティ関数を定義する。

@details C++版と等価な実装を提供する。単に利用するだけならばC++版を推奨する。Python版は独自実装する際の参考としての位置づけである。

"""
from math import atan2, sqrt
import numpy as np
from typing import Any, Iterable, Union
from ASRCAISim1.core import (
    AltitudeKeeper,
    MotionState,
    nljson,
    Track3D,
)

def getExpertDstAZ_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson]
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の左右旋回に関する出力を目標方位角(上方から見て時計回りを正の向きとする)に換算して返す。単位はラジアン。
    
    @details decision["Horizontal"]又はcommand["motion"]["dstDir"]を参照して計算する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
    """
    if myObs.at("isAlive"):
        return 0.0

    originalMyMotion=MotionState(myObs["motion"]) # 元のparent座標系での値も必要

    decisionType=decision["Horizontal"][0]()
    value=decision["Horizontal"][1]()
    motionCmd=command["motion"]()

    if decisionType=="Az_NED":
        dstAZ=value
    elif decisionType=="Az_BODY":
        dstAZ=value+myMotion.getAZ()
    elif "dstDir" in motionCmd:
        dd=originalMyMotion.dirPtoH(motionCmd["dstDir"],originalMyMotion.pos(),"NED",False)
        dstAZ=atan2(dd[1],dd[0])
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDstAZ_PY'.")

    while dstAZ > np.pi:
        dstAZ-=2*np.pi
    while dstAZ <= -np.pi:
        dstAZ+=2*np.pi
    return dstAZ

def getExpertDeltaAZ_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    refAZ: float
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の左右旋回に関する出力を目標方位角(上方から見て時計回りを正の向きとする)と基準方位角の差に換算して返す。単位はラジアン。
    
    @details getExpertDstAZ(...)-refAZを(-π,π]の範囲内に収めたもの。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        refAZ (float): 角度差を計算するための基準方位角。
    """
    deltaAZ = getExpertDstAZ_PY(myObs,myMotion,decision,command)-refAZ
    while deltaAZ > np.pi:
        deltaAZ-=2*np.pi
    while deltaAZ <= -np.pi:
        deltaAZ+=2*np.pi
    return deltaAZ

def getExpertDstPitch_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    altitudeKeeper: Union[AltitudeKeeper, None]
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標ピッチ角(上昇を正の向きとする)に換算して返す。単位はラジアン。
    
    @details decision["Vertical"]又はcommand["motion"]["dstDir"]を参照して計算する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        altitudeKeeper (AltitudeKeeper): 目標高度からピッチ角を計算するために用いるasrc::core::AltitudeKeeper。Noneを与えた場合はデフォルトパラメータで生成したものが用いられる。
    """
    if myObs.at("isAlive"):
        return 0.0

    if altitudeKeeper is None:
        altitudeKeeper=AltitudeKeeper()

    originalMyMotion=MotionState(myObs["motion"]) # 元のparent座標系での値も必要

    decisionType=decision["Vertical"][0]()
    value=decision["Vertical"][1]()
    motionCmd=command["motion"]()

    if decisionType=="El":
        return value
    elif decisionType=="Alt":
        return altitudeKeeper.getDstPitch(myMotion,value)
    elif decisionType=="Pos":
        # Deprecated
        return altitudeKeeper.getDstPitch(myMotion,-value)
    elif "dstDir" in motionCmd:
        dd=originalMyMotion.dirPtoH(motionCmd["dstDir"],originalMyMotion.pos(),"NED",False)
        return -atan2(dd[2],sqrt(dd[0]*dd[0]+dd[1]*dd[1]))
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDstPitch_PY'.")

def getExpertDstAlt_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    altitudeKeeper: Union[AltitudeKeeper, None]
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標高度に換算して返す。単位はm。
    
    @details decision["Vertical"]又はcommand["motion"]["dstDir"]を参照して計算する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        altitudeKeeper (AltitudeKeeper): 目標ピッチ角から目標高度を計算するために用いるasrc::core::AltitudeKeeper。Noneを与えた場合はデフォルトパラメータで生成したものが用いられる。
    """
    if myObs.at("isAlive"):
        return 0.0

    if altitudeKeeper is None:
        altitudeKeeper=AltitudeKeeper()

    originalMyMotion=MotionState(myObs["motion"]) # 元のparent座標系での値も必要

    decisionType=decision["Vertical"][0]()
    value=decision["Vertical"][1]()
    motionCmd=command["motion"]()

    if decisionType=="El":
        return altitudeKeeper.inverse(myMotion,value)
    elif decisionType=="Alt":
        return value
    elif decisionType=="Pos":
        # Deprecated
        return -value
    elif "dstDir" in motionCmd:
        dd=originalMyMotion.dirPtoH(motionCmd["dstDir"],originalMyMotion.pos(),"NED",False)
        dstPitch_ex=-atan2(dd[2],sqrt(dd[0]*dd[0]+dd[1]*dd[1]))
        return altitudeKeeper.inverse(myMotion,dstPitch_ex)
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDstAlt_PY'.")

def getExpertDeltaAlt_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    altitudeKeeper: Union[AltitudeKeeper, None],
    refAlt: float
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標高度に換算して返す。単位はm。
    
    @details decision["Vertical"]又はcommand["motion"]["dstDir"]を参照して計算する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        altitudeKeeper (AltitudeKeeper): 目標ピッチ角から目標高度を計算するために用いるasrc::core::AltitudeKeeper。Noneを与えた場合はデフォルトパラメータで生成したものが用いられる。
        refAlt (float): 高度差を計算するための基準高度。
    """
    if myObs.at("isAlive"):
        return 0.0

    return getExpertDstAlt_PY(myObs,myMotion,decision,command,altitudeKeeper) - refAlt

def getExpertDstV_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    minDeltaV: float,
    maxDeltaV: float,
    minDstAccel: float,
    maxDstAccel: float,
    minDstThrottle: float,
    maxDstThrottle: float,
    asTriChoice: bool = False
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標速度に換算して返す。単位はm/s。
    
    @details getExpertDeltaV(...)-[現在の速度]

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        minDeltaV (float): 考慮する速度増分範囲の下限。
        maxDeltaV (float): 考慮する速度増分範囲の上限。
        minDstAccel (float): 考慮する加速度範囲の下限。
        maxDstAccel (float): 考慮する加速度範囲の上限。
        minDstThrottle (float): 考慮するスロットル範囲の下限。
        maxDstThrottle (float): 考慮するスロットル範囲の上限。
        asTriChoice (bool): 教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(速度増分範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
    """
    if myObs.at("isAlive"):
        return 0.0

    deltaV=getExpertDeltaV_PY(
        myObs,
        myMotion,
        decision,
        command,
        minDeltaV,
        maxDeltaV,
        minDstAccel,
        maxDstAccel,
        minDstThrottle,
        maxDstThrottle,
        asTriChoice
    )
    return np.linalg.norm(myMotion.vel())+deltaV

def getExpertDeltaV_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    minDeltaV: float,
    maxDeltaV: float,
    minDstAccel: float,
    maxDstAccel: float,
    minDstThrottle: float,
    maxDstThrottle: float,
    asTriChoice: bool = False
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標速度と現在速度の差に換算して返す。単位はm/s。
    
    @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
             - 教師役が速度指定の場合はその値と現在速度の差をそのまま用いる。
             - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしてminDeltaV〜maxDeltaVの範囲に線形変換する。
             - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしてminDeltaV〜maxDeltaVの範囲に線形変換する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        minDeltaV (float): 考慮する速度増分範囲の下限。
        maxDeltaV (float): 考慮する速度増分範囲の上限。
        minDstAccel (float): 考慮する加速度範囲の下限。
        maxDstAccel (float): 考慮する加速度範囲の上限。
        minDstThrottle (float): 考慮するスロットル範囲の下限。
        maxDstThrottle (float): 考慮するスロットル範囲の上限。
        asTriChoice (bool): 教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(速度増分範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
    """
    if myObs.at("isAlive"):
        return 0.0

    if maxDeltaV<=minDeltaV:
        raise ValueError("maxDeltaV must be greater than minDeltaV.")

    decisionType=decision["Vertical"][0]()
    value=decision["Vertical"][1]()
    motionCmd=command["motion"]()
    V=np.linalg.norm(myMotion.vel())

    if "dstV" in motionCmd:
        dstV_ex=motionCmd.at("dstV")
        return np.clip(dstV_ex-V,minDeltaV,maxDeltaV)
    elif decisionType=="Vel":
        return np.clip(value-V,minDeltaV,maxDeltaV)
    elif decisionType=="Throttle":
        #0〜1のスロットルで指定していた場合
        if maxDstThrottle<=minDstThrottle:
            raise ValueError("maxDstThrottle must be greater than minDstThrottle.")
        if asTriChoice:
            if value>maxDstThrottle:
                return maxDeltaV
            elif value<minDstThrottle:
                return minDeltaV
            else:
                if minDeltaV>0 or maxDeltaV<0:
                    return (minDeltaV+maxDeltaV)/2
                else:
                    return 0
        else:
            return minDeltaV+(maxDeltaV-minDeltaV)*np.clip(value,minDstThrottle,maxDstThrottle)
    elif decisionType=="Accel":
        #加速度ベースの指定だった場合
        if maxDstAccel<=minDstAccel:
            raise ValueError("maxDstAccel must be greater than minDstAccel.")
        if asTriChoice:
            if value>maxDstAccel:
                return maxDeltaV
            elif value<minDstAccel:
                return minDeltaV
            else:
                if minDeltaV>0 or maxDeltaV<0:
                    return (minDeltaV+maxDeltaV)/2
                else:
                    return 0
        else:
            return minDeltaV+(maxDeltaV-minDeltaV)*np.clip(value,minDstAccel,maxDstAccel)
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDeltaV_PY'.")

def getExpertDstThrottle_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    minDeltaV: float,
    maxDeltaV: float,
    minDstAccel: float,
    maxDstAccel: float,
    minDstThrottle: float,
    maxDstThrottle: float,
    asTriChoice: bool = False
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標スロットル(0〜1)に換算して返す。単位はなし。
    
    @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
             - 教師役が速度指定の場合はその値と現在速度の差をminDeltaV〜maxDeltaVでクリッピングしてminDstThrottle〜maxDstThrottleの範囲に線形変換する。
             - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしてminDstThrottle〜maxDstThrottleの範囲に線形変換する。
             - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしたものとする。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        minDeltaV (float): 考慮する速度増分範囲の下限。
        maxDeltaV (float): 考慮する速度増分範囲の上限。
        minDstAccel (float): 考慮する加速度範囲の下限。
        maxDstAccel (float): 考慮する加速度範囲の上限。
        minDstThrottle (float): 考慮するスロットル範囲の下限。
        maxDstThrottle (float): 考慮するスロットル範囲の上限。
        asTriChoice (bool): 教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、スロットル範囲の中央、最大加速の3値に単純化するか否か。デフォルトはfalse。
    """
    if myObs.at("isAlive"):
        return 0.0

    if maxDstThrottle<=minDstThrottle:
        raise ValueError("maxDstThrottle must be greater than minDstThrottle.")

    decisionType=decision["Vertical"][0]()
    value=decision["Vertical"][1]()
    motionCmd=command["motion"]()
    V=np.linalg.norm(myMotion.vel())

    if "dstV" in motionCmd:
        if maxDeltaV<=minDeltaV:
            raise ValueError("maxDeltaV must be greater than minDeltaV.")

        dstV_ex=motionCmd.at("dstV")
        if asTriChoice:
            if dstV_ex-V>maxDeltaV:
                return maxDstThrottle
            elif dstV_ex-V<minDeltaV:
                return minDstThrottle
            else:
                return (minDstThrottle+maxDstThrottle)/2
        else:
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*np.clip(dstV_ex-V,minDeltaV,maxDeltaV)
    elif decisionType=="Vel":
        if maxDeltaV<=minDeltaV:
            raise ValueError("maxDeltaV must be greater than minDeltaV.")

        if asTriChoice:
            if value-V>maxDeltaV:
                return maxDstThrottle
            elif value-V<minDeltaV:
                return minDstThrottle
            else:
                return (minDstThrottle+maxDstThrottle)/2
        else:
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*np.clip(value-V,minDeltaV,maxDeltaV)
    elif decisionType=="Throttle":
        #0〜1のスロットルで指定していた場合
        return np.clip(value,minDstThrottle,maxDstThrottle)
    elif decisionType=="Accel":
        #加速度ベースの指定だった場合
        if maxDstAccel<=minDstAccel:
            raise ValueError("maxDstAccel must be greater than minDstAccel.")
        if asTriChoice:
            if value>maxDstAccel:
                return maxDstThrottle
            elif value<minDstAccel:
                return minDstThrottle
            else:
                return (minDstThrottle+maxDstThrottle)/2
        else:
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*np.clip(value,minDstAccel,maxDstAccel)
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDstThrottle_PY'.")

def getExpertDstAccel_PY(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    minDeltaV: float,
    maxDeltaV: float,
    minDstAccel: float,
    maxDstAccel: float,
    minDstThrottle: float,
    maxDstThrottle: float,
    asTriChoice: bool = False
) -> float:
    """! @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標加速度に換算して返す。単位はm/s/s。
    
    @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
             - 教師役が速度指定の場合はその値と現在速度の差をminDeltaV〜maxDeltaVでクリッピングしてminDstAccel〜maxDstAccelの範囲に線形変換する。
             - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしたものとする。
             - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしてminDstAccel〜maxDstAccelの範囲に線形変換する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        minDeltaV (float): 考慮する速度増分範囲の下限。
        maxDeltaV (float): 考慮する速度増分範囲の上限。
        minDstAccel (float): 考慮する加速度範囲の下限。
        maxDstAccel (float): 考慮する加速度範囲の上限。
        minDstThrottle (float): 考慮するスロットル範囲の下限。
        maxDstThrottle (float): 考慮するスロットル範囲の上限。
        asTriChoice (bool): 教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(加速度範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
    """
    if myObs.at("isAlive"):
        return 0.0

    if maxDstAccel<=minDstAccel:
        raise ValueError("maxDstAccel must be greater than minDstAccel.")

    decisionType=decision["Vertical"][0]()
    value=decision["Vertical"][1]()
    motionCmd=command["motion"]()
    V=np.linalg.norm(myMotion.vel())

    if "dstV" in motionCmd:
        if maxDeltaV<=minDeltaV:
            raise ValueError("maxDeltaV must be greater than minDeltaV.")

        dstV_ex=motionCmd.at("dstV")
        if asTriChoice:
            if dstV_ex-V>maxDeltaV:
                return maxDstAccel
            elif dstV_ex-V<minDeltaV:
                return minDstAccel
            else:
                if minDstAccel>0 or maxDstAccel<0:
                    return (minDstAccel+maxDstAccel)/2
                else:
                    return 0
        else:
            return minDstAccel+(maxDstAccel-minDstAccel)*np.clip(dstV_ex-V,minDeltaV,maxDeltaV)
    elif decisionType=="Vel":
        if maxDeltaV<=minDeltaV:
            raise ValueError("maxDeltaV must be greater than minDeltaV.")

        if asTriChoice:
            if value-V>maxDeltaV:
                return maxDstAccel
            elif value-V<minDeltaV:
                return minDstAccel
            else:
                if minDstAccel>0 or maxDstAccel<0:
                    return (minDstAccel+maxDstAccel)/2
                else:
                    return 0
        else:
            return minDstAccel+(maxDstAccel-minDstAccel)*np.clip(value-V,minDeltaV,maxDeltaV)
    elif decisionType=="Throttle":
        #0〜1のスロットルで指定していた場合
        if maxDstThrottle<=minDstThrottle:
            raise ValueError("maxDstThrottle must be greater than minDstThrottle.")

        if asTriChoice:
            if value>maxDstThrottle:
                return maxDstAccel
            elif value<minDstThrottle:
                return minDstAccel
            else:
                if minDstAccel>0 or maxDstAccel<0:
                    return (minDstAccel+maxDstAccel)/2
                else:
                    return 0
        else:
            return minDstAccel+(maxDstAccel-minDstAccel)*np.clip(value,minDstThrottle,maxDstThrottle)
    elif decisionType=="Accel":
        #加速度ベースの指定だった場合
        return np.clip(value,minDstAccel,maxDstAccel)
    else:
        raise ValueError("Given unsupported expert's decision & command for the utility function 'getExpertDstAccel_PY'.")

def getExpertShotTarget(
    myObs: Union[dict[str,Any], nljson],
    myMotion: MotionState,
    decision: Union[dict[str,Any], nljson],
    command: Union[dict[str,Any], nljson],
    tracks: Iterable[Track3D]
) -> int:
    """! @brief あるPhysicalAssetに対して教師役が選択した射撃目標を、引数で与えた航跡リストから探してそのインデックスを返す。該当なしの場合は-1を返す。
    
    @details decision["Fire"]を参照して計算する。

    Args:
        myObs (dict | nljson): 対象のPhysicalAssetのobservables。
        myMotion (MotionState): 対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
        decision (dict | nljson): 対象のPhysicalAssetに対する教師役のdecision。
        command (dict | nljson): 対象のPhysicalAssetに対する教師役のcommands。
        tracks (Iterable[Track3D]): 対象とする航跡リスト。
    """
    if myObs.at("isAlive"):
        return 0.0

    if decision["Fire"][0]():
        expertTarget=Track3D(decision["Fire"][1])

        for tIdx,t in enumerate(tracks):
            if t.isSame(expertTarget):
                return tIdx
    return -1
