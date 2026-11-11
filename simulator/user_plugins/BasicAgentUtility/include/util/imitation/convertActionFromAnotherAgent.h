/**
 * Copyright (c) 2026-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief 模倣学習用の行動空間変換を行うためのユーティリティ関数を定義する。
 * 
 */
#pragma once
#include "../../Common.h"
#include <ASRCAISim1/MotionState.h>
#include <ASRCAISim1/FlightControllerUtility.h>

ASRC_PLUGIN_NAMESPACE_BEGIN

namespace util{
    namespace imitation{

/**
 * @brief あるPhysicalAssetに対する教師役の左右旋回に関する出力を目標方位角(上方から見て時計回りを正の向きとする)に換算して返す。単位はラジアン。
 * 
 * @param [in] myObs        対象のPhysicalAssetのobservables。
 * @param [in] myMotion     対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision     対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command      対象のPhysicalAssetに対する教師役のcommands。
 * 
 * @details decision["Horizontal"]又はcommand["motion"]["dstDir"]を参照して計算する。
 * 
 */
double PYBIND11_EXPORT getExpertDstAZ(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command
);

/**
 * @brief あるPhysicalAssetに対する教師役の左右旋回に関する出力を目標方位角(上方から見て時計回りを正の向きとする)と基準方位角の差に換算して返す。単位はラジアン。
 * 
 * @param [in] myObs        対象のPhysicalAssetのobservables。
 * @param [in] myMotion     対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision     対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command      対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] refAZ        角度差を計算するための基準方位角。
 * 
 * @details getExpertDstAZ(...)-refAZを(-π,π]の範囲内に収めたもの。
 * 
 */
double PYBIND11_EXPORT getExpertDeltaAZ(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& refAZ
);

/**
 * @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標ピッチ角(上昇を正の向きとする)に換算して返す。単位はラジアン。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] altitudeKeeper   目標高度からピッチ角を計算するために用いるasrc::core::AltitudeKeeperへのポインタ。nullptrを与えた場合はデフォルトパラメータで生成したものが用いられる。
 * 
 * @details decision["Vertical"]又はcommand["motion"]["dstDir"]を参照して計算する。
 * 
 */
double PYBIND11_EXPORT getExpertDstPitch(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper
);

/**
 * @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標高度に換算して返す。単位はm。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] altitudeKeeper   目標ピッチ角から目標高度を計算するために用いるasrc::core::AltitudeKeeperへのポインタ。nullptrを与えた場合はデフォルトパラメータで生成したものが用いられる。
 * 
 * @details decision["Vertical"]又はcommand["motion"]["dstDir"]を参照して計算する。
 * 
 */
double PYBIND11_EXPORT getExpertDstAlt(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper
);

/**
 * @brief あるPhysicalAssetに対する教師役の上昇・下降に関する出力を目標高度と基準高度の差に換算して返す。単位はm。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] altitudeKeeper   目標ピッチ角から目標高度を計算するために用いるasrc::core::AltitudeKeeperへのポインタ。nullptrを与えた場合はデフォルトパラメータで生成したものが用いられる。
 * @param [in] refAlt           高度差を計算するための基準高度。
 * 
 * @details getExpertDstAlt(...)-refAlt
 * 
 */
double PYBIND11_EXPORT getExpertDeltaAlt(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper,
    const double& refAlt
);

/**
 * @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標速度に換算して返す。単位はm/s。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] minDeltaV        考慮する速度増分範囲の下限。
 * @param [in] maxDeltaV        考慮する速度増分範囲の上限。
 * @param [in] minDstAccel      考慮する加速度範囲の下限。
 * @param [in] maxDstAccel      考慮する加速度範囲の上限。
 * @param [in] minDstThrottle   考慮するスロットル範囲の下限。
 * @param [in] maxDstThrottle   考慮するスロットル範囲の上限。
 * @param [in] asTriChoice      教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(速度増分範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
 * 
 * @details getExpertDeltaV(...)-[現在の速度]
 */
double PYBIND11_EXPORT getExpertDstV(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice=false
);

/**
 * @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標速度と現在速度の差に換算して返す。単位はm/s。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] minDeltaV        考慮する速度増分範囲の下限。
 * @param [in] maxDeltaV        考慮する速度増分範囲の上限。
 * @param [in] minDstAccel      考慮する加速度範囲の下限。
 * @param [in] maxDstAccel      考慮する加速度範囲の上限。
 * @param [in] minDstThrottle   考慮するスロットル範囲の下限。
 * @param [in] maxDstThrottle   考慮するスロットル範囲の上限。
 * @param [in] asTriChoice      教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(速度増分範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
 * 
 * @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
 *          - 教師役が速度指定の場合はその値と現在速度の差をそのまま用いる。
 *          - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしてminDeltaV〜maxDeltaVの範囲に線形変換する。
 *          - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしてminDeltaV〜maxDeltaVの範囲に線形変換する。
 */
double PYBIND11_EXPORT getExpertDeltaV(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice=false
);

/**
 * @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標スロットル(0〜1)に換算して返す。単位はなし。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] minDeltaV        考慮する速度増分範囲の下限。
 * @param [in] maxDeltaV        考慮する速度増分範囲の上限。
 * @param [in] minDstAccel      考慮する加速度範囲の下限。
 * @param [in] maxDstAccel      考慮する加速度範囲の上限。
 * @param [in] minDstThrottle   考慮するスロットル範囲の下限。
 * @param [in] maxDstThrottle   考慮するスロットル範囲の上限。
 * @param [in] asTriChoice      教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、スロットル範囲の中央、最大加速の3値に単純化するか否か。デフォルトはfalse。
 * 
 * @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
 *          - 教師役が速度指定の場合はその値と現在速度の差をminDeltaV〜maxDeltaVでクリッピングしてminDstThrottle〜maxDstThrottleの範囲に線形変換する。
 *          - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしてminDstThrottle〜maxDstThrottleの範囲に線形変換する。
 *          - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしたものとする。
 */
double PYBIND11_EXPORT getExpertDstThrottle(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice=false
);

/**
 * @brief あるPhysicalAssetに対する教師役の加減速に関する出力を目標加速度に換算して返す。単位はm/s/s。
 * 
 * @param [in] myObs            対象のPhysicalAssetのobservables。
 * @param [in] myMotion         対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision         対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command          対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] minDeltaV        考慮する速度増分範囲の下限。
 * @param [in] maxDeltaV        考慮する速度増分範囲の上限。
 * @param [in] minDstAccel      考慮する加速度範囲の下限。
 * @param [in] maxDstAccel      考慮する加速度範囲の上限。
 * @param [in] minDstThrottle   考慮するスロットル範囲の下限。
 * @param [in] maxDstThrottle   考慮するスロットル範囲の上限。
 * @param [in] asTriChoice      教師役の値を直接使用できず変換が必要な場合に、線形変換でなく最大減速、等速(加速度範囲が0を含まない場合は範囲の中央)、最大加速の3値に単純化するか否か。デフォルトはfalse。
 * 
 * @details decision["Throttle"]又はcommand["motion"]["dstV"]を参照して計算する。
 *          - 教師役が速度指定の場合はその値と現在速度の差をminDeltaV〜maxDeltaVでクリッピングしてminDstAccel〜maxDstAccelの範囲に線形変換する。
 *          - 教師役が加速度指定の場合は、minDstAccel〜maxDstAccelでクリッピングしたものとする。
 *          - 教師役がスロットル指定の場合は、minDstThrottle〜maxDstThrottleでクリッピングしてminDstAccel〜maxDstAccelの範囲に線形変換する。
 */
double PYBIND11_EXPORT getExpertDstAccel(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice=false
);

/**
 * @brief あるPhysicalAssetに対して教師役が選択した射撃目標を、引数で与えた航跡リストから探してそのインデックスを返す。該当なしの場合は-1を返す。
 * 
 * @param [in] myObs        対象のPhysicalAssetのobservables。
 * @param [in] myMotion     対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision     対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command      対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] tracks       対象とする航跡リスト。
 * 
 * @details decision["Fire"]を参照して計算する。
 */
int PYBIND11_EXPORT getExpertShotTarget(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const std::vector<asrc::core::Track3D>& tracks
);

/**
 * @brief あるPhysicalAssetに対して教師役が選択した射撃目標を、引数で与えた航跡リストから探してそのインデックスを返す。該当なしの場合は-1を返す。
 * 
 * @param [in] myObs        対象のPhysicalAssetのobservables。
 * @param [in] myMotion     対象のPhysicalAssetのMotionStateをAgentのlocalCRSに座標変換したもの。
 * @param [in] decision     対象のPhysicalAssetに対する教師役のdecision。
 * @param [in] command      対象のPhysicalAssetに対する教師役のcommands。
 * @param [in] tracks       対象とする航跡リスト。
 * 
 * @details decision["Fire"]を参照して計算する。
 */
int PYBIND11_EXPORT getExpertShotTarget(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const py::list& tracks
);

void exportConvertActionFromAnotherAgent(py::module &m);

    }
}

ASRC_PLUGIN_NAMESPACE_END
