// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include <functional>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/CXX11/Tensor>
#include "MathUtility.h"
#include "util/TableData.h"
#include "FlightControllerUtility.h"
#include "Fighter.h"
#include "Utility.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

/**
 * @class SimpleFighterJetEngine
 * @brief F-16のエンジンを模したモデル。文献[1]〜[3]よりモデル化。
 * 
 * @details
 *      F-16 engine model in [1]. Supersonic max thrust value is obtained from a figure in [2].
 *      Fuel consumption is based on the fuel flow rate in [3].
 *      
 *      References:
 *      [1] Stevens, Brian L., et al. "Aircraft control and simulation: dynamics, controls design, and autonomous systems."" John Wiley & Sons, 2015.
 *      [2] Henrdrick, P., et al. "A Flight Thrust Deck for the F100 Turbofan of the F-16 Aircraft." 26th Congress of International Council of the Aeronautical Sciences, 2008.
 *      [3] Air Force Civil Engineer Center. "Air Emission Guide for Air Force Mobile Sources." Air Force Civil Engineer Center, 2013.
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(SimpleFighterJetEngine,Propulsion)
    public:
    bool enableInstantResponse;
    bool considerFuel;
    double tHalf;
    TableData<double,2> tminTable,tmilTable,tmaxTable;// In [lbf]
    double fuelFlowRateMin,fuelFlowRateMil,fuelFlowRateMax;// [lb/hours]
    //engine states
    double thrust;
    double power;
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
    double calcThrust(double pCmd,double alt,double rmach) const;
    double trimPower(double tgtThrust,double alt,double Mach) const;
    double calcPCmdFromThrottle(double throttle) const;
    double calcdTdP(double power_,double alt,double rmach) const;
    double calcdTdH(double power_,double alt,double rmach) const;
    double calcdTdM(double power_,double alt,double rmach) const;
    virtual double rtau(double dp) const;
    virtual double pdot(double pCurr_,double pCmd_) const;
    virtual void behave() override;
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(SimpleFighterJetEngine)
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
 * @class CoordinatedFighter
 * @brief 横滑り角を0に拘束した簡易航空機モデル。F-16相当の挙動を文献[Stevens 2015, Krus 2019, Webb 1977]よりモデル化。
 * 
 * @details
 *      Point mass approximation assuming "coordinated" flight.
 *      Aerodynamic coefficients and engine characteristics are based on public released F-16 information listed below.
 *      Most of the model is based on [Stevens 2015], wave drag is based on [krus 2019], and supersonic CD0 is based on [Webb 1977].
 * 
 *      References:
 *      [Stevens 2015] Stevens, Brian L., et al. "Aircraft control and simulation: dynamics, controls design, and autonomous systems." John Wiley & Sons, 2015.
 *      [Krus 2019] Krus, P., et al. "Modelling of Transonic and Supersonic Aerodynamics for Conceptual Design and Flight Simulation." Proceedings of the 10th Aerospace Technology Congress, 2019.
 *      [Webb 1977] Webb, T. S., et al. "Correlation of F-16 Aerodynamics and Performance Predictions with Early Flight Test Results." Agard Conference Proceedings, N242, 1977.
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(CoordinatedFighter,Fighter)
    public:
    //model parameters
    double S,rollMax,sideGLimit;
    TableData<double,2> cxTable,cmTable;
    TableData<double,1> czTable,trimmedDe;
    Eigen::VectorXd cdwTable;
    double minAoa,maxAoa,maxCz,minCz;
    public:
    //constructors & destructor
    CoordinatedFighter(const nl::json& modelConfig_,const nl::json& instanceConfig_);
    //functions
    virtual void initialize() override;
    virtual void makeChildren() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual double calcOptimumCruiseFuelFlowRatePerDistance() override;;//Minimum fuel flow rate (per distance) in level steady flight
    virtual double trimAoa(double tgtCz);
    virtual double cm(double aoa_,double de_);
    virtual double cmtrim(double aoa_);
    virtual double cx(double aoa_);
    virtual double cz(double aoa_);
    virtual double cxa(double aoa_);
    virtual double cza(double aoa_);
    virtual double cl(double aoa_);
    virtual double cd(double aoa_,double mach_);
    virtual double cdw(double mach_);
    virtual Eigen::Vector3d calcAero(double aoa_,double mach_);
    virtual void calcMotion(double dt) override;
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(CoordinatedFighter)
    virtual double calcOptimumCruiseFuelFlowRatePerDistance() override{
        PYBIND11_OVERRIDE(double,Base,calcOptimumCruiseFuelFlowRatePerDistance);
    }
    virtual void calcMotion(double dt) override{
        PYBIND11_OVERRIDE(void,Base,calcMotion,dt);
    }
    virtual double trimAoa(double tgtCz) override{
        PYBIND11_OVERRIDE(double,Base,trimAoa,tgtCz);
    }
    virtual double cm(double aoa_,double de_) override{
        PYBIND11_OVERRIDE(double,Base,cm,aoa_,de_);
    }
    virtual double cmtrim(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cmtrim,aoa_);
    }
    virtual double cx(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cx,aoa_);
    }
    virtual double cz(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cz,aoa_);
    }
    virtual double cxa(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cxa,aoa_);
    }
    virtual double cza(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cza,aoa_);
    }
    virtual double cl(double aoa_) override{
        PYBIND11_OVERRIDE(double,Base,cl,aoa_);
    }
    virtual double cd(double aoa_,double mach_) override{
        PYBIND11_OVERRIDE(double,Base,cd,aoa_,mach_);
    }
    virtual double cdw(double mach_) override{
        PYBIND11_OVERRIDE(double,Base,cdw,mach_);
    }
    virtual Eigen::Vector3d calcAero(double aoa_,double mach_) override{
        PYBIND11_OVERRIDE(Eigen::Vector3d,Base,calcAero,aoa_,mach_);
    }
};

/**
 * @class CoordinatedFlightController
 * @brief  CoordinatedFighter用の簡易的な飛行制御モデル
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(CoordinatedFlightController,FlightController)
    public:
    double lambdaVel,lambdaTheta,clampTheta;
    double kVel,kAccel,kPower,kTheta,kOmega,kAoa,kEps;
    double pitchLimit,pitchLimitThreshold;
    AltitudeKeeper altitudeKeeper;
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual nl::json getDefaultCommand() override;
    virtual nl::json calc(const nl::json &cmd) override;
    nl::json calcDirect(const nl::json &cmd);
    nl::json calcFromDirAndVel(const nl::json &cmd);
};

void exportCoordinatedFighter(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
