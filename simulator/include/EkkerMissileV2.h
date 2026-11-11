// Copyright (c) 2023-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include "EkkerMissile.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

/**
 * @class EkkerMissileV2
 * @brief　文献[Ekker 1994]に示されている空力モデルを、[Pitts 1957](NACA-TR-1307)に基づいてテーパ翼と前翼での制御に対応させた誘導弾モデル。
 * 
 * @details
 *      Based on [Ekker 1994] Appendix. B, but modified a bit.
 *      (1) All values should be provided in MKS units. (angles should be in rad.)
 *      (2) Transonic region (M=0.95 to 1.2) is also calculated by same method as either sub/supersonic region to simplify, although it is not so accuarate.
 *      (3) Extended to tapered planform and canard-controlled missiles based on [Pitts 1957](NACA-TR-1307)
 *      (4) Induced drag on tail is calculated with the effect of downwash, which [Ekker 1994] seems to be ignore by dε/dα≒1.
 *      
 *      References:
 *      [Ekker 1994] Ekker, David A. "Missile Design Toolbox." Diss. Monterey, California. Naval Postgraduate School, 1994.
 *      [Pitts 1957] Pitts, William C., et al. "Lift and Center of Pressure of Wing-Body-Tail Combinations at Subsonic, Transonic, and Supersonic Speeds." NACA-TR-1307, 1957.
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(EkkerMissileV2,EkkerMissile)
    public:
    //configで指定するもの
    double taperRatioW,taperRatioT;
    bool isTailControlled;
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual std::pair<double,Eigen::Vector3d> calcAccel(const Eigen::Vector3d& accelCmd, double tAftLaunch,double dt); //!< x軸方向の加速度と回転角速度の組を計算する。
};

void exportEkkerMissileV2(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
