// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include <iostream>
#include <unordered_map>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include "MathUtility.h"
#include "Sensor.h"
#include "Utility.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

/**
 * @class AircraftRadarV2
 * @brief 探知確率、誤差ありのレーダを表すクラス
 * 
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(AircraftRadarV2,AircraftRadar)
    public:
    //parameters
    double pfa,pl,angleErrorMean,angleErrorSigma,rangeErrorMean,rangeErrorSigma,rVelErrorMean,rVelErrorSigma,cVelErrorMean,cVelErrorSigma;
    int nTry,nNeedDetect,nMemory,nNeedMemory;
    bool lookRCSScale;
    //internal variables
    std::vector<Track3D> candidates;
    std::vector<Eigen::VectorXi> memoryCount,candidatesMemoryCount;
    double snCoeff;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void perceive(bool inReset) override;
};

/**
 * @class IRSTV2
 * @brief 誤差ありのIRセンサを表すクラス
 * 
 * @details
 *      真の方向からdAngle[rad]傾いた方向に航跡を生成する。dAngleは正規分布N(0,angleErrorSigma)でサンプリングする。
 *      どの方向に傾けるかは[-pi,pi)の一様分布でサンプリングする。
 * 
 *      航跡の方向変化率については、ベクトルの大きさとベクトルを傾ける角度の2つを正規分布でサンプリングする。
 *      どの方向に傾けるかは[-pi,pi)の一様分布でサンプリングする。
 *      
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(IRSTV2,IRST)
    public:
    //parameters
    double angleErrorSigma; //!< 角度誤差の標準偏差[deg]
    double omegaErrorSigma; //!< 方向変化率の大きさの標準偏差[deg]
    double omegaDirectionErrorSigma; //!< 方向変化率ベクトルの角度誤差の標準偏差[deg]

    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void perceive(bool inReset) override;
};

/**
 * @class MWSV2
 * @brief 誤差ありのMWSを表すクラス
 * 
 * @details
 *      真の方向からdAngle[rad]傾いた方向に航跡を生成する。dAngleは正規分布N(0,angleErrorSigma)でサンプリングする。
 *      どの方向に傾けるかは[-pi,pi)の一様分布でサンプリングする。
 * 
 *      航跡の方向変化率については、ベクトルの大きさとベクトルを傾ける角度の2つを正規分布でサンプリングする。
 *      どの方向に傾けるかは[-pi,pi)の一様分布でサンプリングする。
 *      
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(MWSV2,MWS)
    public:
    //parameters
    double angleErrorSigma; //!< 角度誤差の標準偏差[deg]
    double omegaErrorSigma; //!< 方向変化率の大きさの標準偏差[deg]
    double omegaDirectionErrorSigma; //!< 方向変化率ベクトルの角度誤差の標準偏差[deg]

    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void perceive(bool inReset) override;
};

/**
 * @class MissileSensorV2
 * @brief 誤差ありの誘導弾センサ(シーカ)を表すクラス
 *      
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(MissileSensorV2,MissileSensor)
    public:
    //parameters
    double angleErrorMean,angleErrorSigma,rangeErrorMean,rangeErrorSigma,rVelErrorMean,rVelErrorSigma,cVelErrorMean,cVelErrorSigma;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void perceive(bool inReset) override;
};

void exportSensorV2(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
