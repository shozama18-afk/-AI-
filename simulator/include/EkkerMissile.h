// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include <Eigen/CXX11/Tensor>
#include "MathUtility.h"
#include "Missile.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

/**
 * @class EkkerMissile
 * @brief　文献[Ekker 1994]に示されている空力モデルを用いた誘導弾モデル。
 * 
 * @details
 *      Based on [Ekker 1994] Appendix. B, but modified a bit.
 *      (1) All values should be provided in MKS units. (angles should be in rad.)
 *      (2) Transonic region (M=0.95 to 1.2) is also calculated by same method as either sub/supersonic region to simplify, although it is not so accuarate.
 *      
 *      References:
 *      [Ekker 1994] Ekker, David A. "Missile Design Toolbox." Diss. Monterey, California. Naval Postgraduate School, 1994.
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(EkkerMissile,Missile)
    public:
    //configで指定するもの
    double tMax,tBurn,hitD,minV;
    double massI,massF,thrust,maxLoadG;
    double Sref,maxA,maxD,l,d,ln,lcgi,lcgf,lw,bw,bt,thicknessRatio,Sw,St;
    double frictionFactor;
    //内部変数
    double mass;//質量
    double lcg;//重心位置
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void calcMotion(double tAftLaunch,double dt) override;
    virtual std::pair<double,Eigen::Vector3d> calcAccel(const Eigen::Vector3d& accelCmd, double tAftLaunch,double dt); //!< x軸方向の加速度と回転角速度の組を計算する。
    virtual bool hitCheck(const Eigen::Vector3d &tpos,const Eigen::Vector3d &tpos_prev) override;
    virtual bool checkDeactivateCondition(double tAftLaunch) override;
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(EkkerMissile)
    virtual void calcMotion(double tAftLaunch,double dt) override{
        PYBIND11_OVERRIDE(void,Base,calcMotion,tAftLaunch,dt);
    }
    virtual std::pair<double,Eigen::Vector3d> calcAccel(const Eigen::Vector3d& accelCmd, double tAftLaunch,double dt) override{
        using ReturnType=std::pair<double,Eigen::Vector3d>;
        PYBIND11_OVERRIDE(ReturnType,Base,calcAccel,accelCmd,tAftLaunch,dt);
    }
    virtual bool hitCheck(const Eigen::Vector3d &tpos,const Eigen::Vector3d &tpos_prev) override{
        PYBIND11_OVERRIDE(bool,Base,hitCheck,tpos,tpos_prev);
    }
    virtual bool checkDeactivateCondition(double tAftLaunch) override{
        PYBIND11_OVERRIDE(bool,Base,checkDeactivateCondition,tAftLaunch);
    }
};

void exportEkkerMissile(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
