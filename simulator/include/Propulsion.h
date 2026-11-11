// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include "PhysicalAsset.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

/**
 * @class Propulsion
 * @brief 燃料を噴射して推力を得る推進器(ジェットエンジン、ロケットエンジン)を表すベースクラス
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(Propulsion,PhysicalAsset)
	/*
	Generic Propulsion base class.
    */
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual double getFuelFlowRate() const=0; //!< 現時点の燃料流量[kg/s]を取得する。
    virtual double getThrust() const=0; //!< 現時点の推力[N]を取得する。
    virtual double calcFuelFlowRate(const nl::json& args) const=0; //!< 引数の状態の燃料流量[kg/s]を計算する。(as generic interface)
    virtual double calcThrust(const nl::json& args) const=0; //!< 引数の状態の推力[N]を計算する。(as generic interface)
    virtual void setPowerCommand(const double& pCmd_)=0; //!< 0〜1で正規化された推力コマンドを設定する。(as generic interface)
    virtual void control() override;
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(Propulsion)
    virtual double getFuelFlowRate() const override{
        PYBIND11_OVERRIDE_PURE(double,Base,getFuelFlowRate);
    }
    virtual double getThrust() const override{
        PYBIND11_OVERRIDE_PURE(double,Base,getThrust);
    }
    virtual double calcFuelFlowRate(const nl::json& args) const override{
        PYBIND11_OVERRIDE_PURE(double,Base,calcFuelFlowRate,args);
    }
    virtual double calcThrust(const nl::json& args) const override{
        PYBIND11_OVERRIDE_PURE(double,Base,calcThrust,args);
    }
    virtual void setPowerCommand(const double& pCmd_) override{
        PYBIND11_OVERRIDE_PURE(void,Base,setPowerCommand,pCmd_);
    }
};

void exportPropulsion(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
