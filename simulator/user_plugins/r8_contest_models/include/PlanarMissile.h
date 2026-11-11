/**
 * Copyright (c) 2025-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief R8年度コンテストユース部門の誘導弾モデル。オープン部門の誘導弾モデルの運動を2次元平面上に拘束したもの。
 */
#pragma once
#include <ASRCAISim1/EkkerMissile.h>
#include "Common.h"

ASRC_PLUGIN_NAMESPACE_BEGIN

/**
 * @class PlanarMissile
 * @brief EkkerMissileの運動を2次元平面上に拘束した誘導弾モデル。
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(PlanarMissile,asrc::core::EkkerMissile)
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void calcMotion(double tAftLaunch,double dt) override;
    virtual std::pair<double,Eigen::Vector3d> calcAccel(const Eigen::Vector3d& accelCmd, double tAftLaunch,double dt) override; //!< x軸方向の加速度と回転角速度の組を計算する。

    //射程テーブルの計算(自身と目標が同一平面上に存在する場合しか計算できないため、htを無視して彼我ともにhsを用いる)
    virtual bool calcRangeSub(double vs,double hs,double vt,double ht,double obs,double aa,double r) override;
    virtual void makeRangeTable(const std::string& dstPath) override;
};

void exportPlanarMissile(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_PLUGIN_NAMESPACE_END
