// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include <deque>
#include <vector>
#include <functional>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include "MathUtility.h"
#include "FlightControllerUtility.h"
#include "Fighter.h"
#include "Controller.h"
#include "Track.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class Missile;
class FighterSensor;

/**
 * @class IdealDirectPropulsion
 * @brief 入力コマンドに比例する推力を遅延なく発生させる最も単純な推進システムを表すクラス。燃料消費も考慮しない。
 * 
 * @details
 *          An ideal, simplest propulsion model linear acceleration without fuel consumption
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(IdealDirectPropulsion,Propulsion)
    public:
    double tMin,tMax;//Min. & Max. thrust in [N]
    //engine states
    double thrust;
    double pCmd;

    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual double getFuelFlowRate() const override; //!< 現時点の燃料流量[kg/s]を取得する。
    virtual double getThrust() const override; //!< 現時点の推力[N]を取得する。
    virtual double calcFuelFlowRate(const nl::json& args) const override; //!< 引数の状態の燃料流量[kg/s]を計算する。(as generic interface)
    virtual double calcThrust(const nl::json& args) const override; //!< 引数の状態の推力[N]を計算する。(as generic interface)
    virtual void setPowerCommand(const double& pCmd_) override; //!< 0〜1で正規化された推力コマンドを設定する。(as generic interface)
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(IdealDirectPropulsion)
    virtual double getFuelFlowRate() const override{
        PYBIND11_OVERRIDE(double,Base,getFuelFlowRate);
    }
    virtual double getThrust() const override{
        PYBIND11_OVERRIDE(double,Base,getThrust);
    }
    virtual double calcFuelFlowRate(const nl::json& args) const override{
        PYBIND11_OVERRIDE(double,Base,calcFuelFlowRate,args);
    }
    virtual double calcThrust(const nl::json& args) const override{
        PYBIND11_OVERRIDE(double,Base,calcThrust,args);
    }
    virtual void setPowerCommand(const double& pCmd_) override{
        PYBIND11_OVERRIDE(void,Base,setPowerCommand,pCmd_);
    }
};

/**
 * @class MassPointFighter
 * @brief 重力や空気力を無視し速度と角速度を一定の範囲で直接操作できるようにした戦闘機モデル
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(MassPointFighter,Fighter)
    public:
    //model parameters
    double vMin,vMax,rollMax,pitchMax,yawMax;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual double calcOptimumCruiseFuelFlowRatePerDistance() override;//Minimum fuel flow rate (per distance) in level steady flight
    virtual void calcMotion(double dt) override;
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(MassPointFighter)
    virtual double calcOptimumCruiseFuelFlowRatePerDistance() override{
        PYBIND11_OVERRIDE(double,Base,calcOptimumCruiseFuelFlowRatePerDistance);
    }
    virtual void calcMotion(double dt) override{
        PYBIND11_OVERRIDE(void,Base,calcMotion,dt);
    }
};

/**
 * @class MassPointFlightController
 * @brief  MassPointFighter用の簡易的な飛行制御モデル
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(MassPointFlightController,FlightController)
    public:
    //model parameters
    AltitudeKeeper altitudeKeeper;
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual nl::json getDefaultCommand() override;
    virtual nl::json calc(const nl::json &cmd) override;
    virtual nl::json calcDirect(const nl::json &cmd);
    virtual nl::json calcFromDirAndVel(const nl::json &cmd);
};

void exportMassPointFighter(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
