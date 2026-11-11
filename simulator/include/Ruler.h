/**
 * Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief シミュレーションのルールを定義するクラス
 */
#pragma once
#include <vector>
#include <map>
#include <pybind11/pybind11.h>
#include "Callback.h"

namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class Agent;
class Asset;
class RulerAccessor;

/**
 * @class Ruler
 * @brief シミュレーションのルールを定義するクラス
 * 
 * @details エピソードの終了判定の実施と各陣営の得点の計算を主な役割とする。
 *          一つのエピソード中に一つしか存在できない。
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(Ruler,Callback)
    public:
    static const std::string baseName; // baseNameを上書き
    enum class EndReason{
        TIMEUP,
        NO_ONE_ALIVE,
        NOTYET
    };
    std::map<std::string,bool> dones; //!< 各 Agent の終了状況
    double maxTime;
    std::vector<std::string> teams;
    std::string winner;
    EndReason endReason;
    std::map<std::string,double> score; //!< 各陣営の累積得点。
    std::map<std::string,double> stepScore; //!< 各陣営のこのstep
    bool independentPenalty; //!< 失格判定用のペナルティを独立に計算するかどうか。
    std::map<std::string,double> totalPenalty; //!< 各陣営の累積ペナルティ(独立計算時のみ使用)
    std::map<std::string,double> stepPenalty; //!< 各陣営のこのstepにおけるペナルティ(独立計算時のみ使用)
    nl::json observables; //!< 他者が観測してよい自身の情報
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void onEpisodeBegin() override;
    virtual void onStepBegin() override;
    virtual void onInnerStepBegin() override;
    virtual void onInnerStepEnd() override;
    virtual void onStepEnd() override;
    virtual void onEpisodeEnd() override;
    virtual void checkDone(); //!< 各 Agent の終了状況を判定する。
    virtual std::string getEndReason() const; //!< 終了理由を表す文字列を返す。
    virtual void setEndReason(const std::string& newReason); //!< 終了理由を表す文字列を変更する。
    virtual double getStepScore(const std::string &key); //!< key をfull nameとする Agent のこのstepにおける獲得得点を返す。
    virtual double getScore(const std::string &key); //!< key をfull nameとする Agent のこれまでの獲得得点を返す。
    virtual double getStepScore(const std::shared_ptr<Agent>& key); //!< key のこのstepにおける獲得得点を返す。
    virtual double getScore(const std::shared_ptr<Agent>& key); //!< key のこれまでの獲得得点を返す。
    virtual double getStepPenalty(const std::string &key); //!< key をfull nameとする Agent のこのstepにおけるペナルティを返す。(独立計算時のみ有効)
    virtual double getTotalPenalty(const std::string &key); //!< key をfull nameとする Agent のこれまでの累積ペナルティを返す。(独立計算時のみ有効)
    virtual double getStepPenalty(const std::shared_ptr<Agent>& key); //!< key のこのstepにおけるペナルティを返す。(独立計算時のみ有効)
    virtual double getTotalPenalty(const std::shared_ptr<Agent>& key); //!< key のこれまでの累積ペナルティを返す。(独立計算時のみ有効)
    virtual std::shared_ptr<CoordinateReferenceSystem> getLocalCRS() const; //!< ルールを表現するための基準となるローカル座標系を返す。初回呼び出し時にメンバ変数localCRSの値を設定する。派生クラスでカスタマイズしてよい。
    virtual std::shared_ptr<EntityAccessor> getAccessorImpl() override;
    protected:
    std::shared_ptr<CoordinateReferenceSystem> localCRS; //!< ルールを表現するための基準となるローカル座標系。直接ではなくgetLocalCRS()で参照することを想定。省略時は manager の rootCRS を使用する。
};
DEFINE_SERIALIZE_ENUM_AS_STRING(Ruler::EndReason)

/**
 * @class RulerAccessor
 * @brief Ruler とその派生クラスのメンバに対するアクセス制限を実現するためのAccessorクラス。
 * 
 * @details Agent 用
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(RulerAccessor,EntityAccessor)
    public:
    RulerAccessor(const std::shared_ptr<Ruler>& r);
    virtual double getStepScore(const std::string &key); //!< key をfull nameとする Agent のこのstepにおける獲得得点を返す。
    virtual double getScore(const std::string &key); //!< key をfull nameとする Agent のこれまでの獲得得点を返す。
    virtual double getStepScore(const std::shared_ptr<Agent>& key); //!< key のこのstepにおける獲得得点を返す。
    virtual double getScore(const std::shared_ptr<Agent>& key); //!< key のこれまでの獲得得点を返す。
    virtual double getStepPenalty(const std::string &key); //!< key をfull nameとする Agent のこのstepにおけるペナルティを返す。(独立計算時のみ有効)
    virtual double getTotalPenalty(const std::string &key); //!< key をfull nameとする Agent のこれまでの累積ペナルティを返す。(独立計算時のみ有効)
    virtual double getStepPenalty(const std::shared_ptr<Agent>& key); //!< key のこのstepにおけるペナルティを返す。(独立計算時のみ有効)
    virtual double getTotalPenalty(const std::shared_ptr<Agent>& key); //!< key のこれまでの累積ペナルティを返す。(独立計算時のみ有効)
    virtual std::shared_ptr<CoordinateReferenceSystem> getLocalCRS() const; //!< ルールを表現するための基準となるローカル座標系を返す。
    const nl::json& observables; //!< 自身のobservablesのconst参照。Python側からもread-onlyプロパティとして参照可能
    protected:
    static nl::json dummy;
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(Ruler)
    virtual void checkDone() override{
        PYBIND11_OVERRIDE(void,Base,checkDone);
    }
    virtual std::string getEndReason() const override{
        PYBIND11_OVERRIDE(std::string,Base,getEndReason);
    }
    virtual void setEndReason(const std::string& newReason) override{
        PYBIND11_OVERRIDE(void,Base,setEndReason,newReason);
    }
    virtual double getStepScore(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getStepScore,key);
    }
    virtual double getScore(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getScore,key);
    }
    virtual double getStepScore(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getStepScore,agent);
    }
    virtual double getScore(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getScore,agent);
    }
    virtual double getStepPenalty(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getStepPenalty,key);
    }
    virtual double getTotalPenalty(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getTotalPenalty,key);
    }
    virtual double getStepPenalty(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getStepPenalty,agent);
    }
    virtual double getTotalPenalty(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getTotalPenalty,agent);
    }
    virtual std::shared_ptr<CoordinateReferenceSystem> getLocalCRS() const override{
        PYBIND11_OVERRIDE(std::shared_ptr<CoordinateReferenceSystem>,Base,getLocalCRS);
    }
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(RulerAccessor)
    virtual double getStepScore(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getStepScore,key);
    }
    virtual double getScore(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getScore,key);
    }
    virtual double getStepScore(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getStepScore,agent);
    }
    virtual double getScore(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getScore,agent);
    }
    virtual double getStepPenalty(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getStepPenalty,key);
    }
    virtual double getTotalPenalty(const std::string &key) override{
        PYBIND11_OVERRIDE(double,Base,getTotalPenalty,key);
    }
    virtual double getStepPenalty(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getStepPenalty,agent);
    }
    virtual double getTotalPenalty(const std::shared_ptr<Agent>& agent) override{
        PYBIND11_OVERRIDE(double,Base,getTotalPenalty,agent);
    }
    virtual std::shared_ptr<CoordinateReferenceSystem> getLocalCRS() const override{
        PYBIND11_OVERRIDE(std::shared_ptr<CoordinateReferenceSystem>,Base,getLocalCRS);
    }
};

void exportRuler(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
