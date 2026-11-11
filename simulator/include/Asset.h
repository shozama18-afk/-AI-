/**
 * Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief シミュレーションで模擬される対象物を表すクラス
 * 
 * @details 模擬対象物を表す Asset とそのアクセス制限を担う AssetAccessor からなる。
 */
#pragma once
#include <map>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include "Quaternion.h"
#include "MathUtility.h"
#include "StructuredEntity.h"
#include "CommunicationBuffer.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class SimulationManagerAccessorBase;
class Agent;
class Controller;
class AssetAccessor;

/**
 * @class Asset
 * @brief シミュレーションで模擬される対象物を表すクラス
 * 
 * @details Asset は、観測(perceive)、制御(control)、行動(behave)の 3 種類の処理をそれぞれ指定された周期で実行することで環境に作用する。
 *          また、各 Asset の処理の中には他の Asset の処理結果に依存するものがあるため、同時刻に処理を行うこととなった際の処理優先度を
 *          setDependency 関数によって指定することができる。
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(Asset,StructuredEntity)
    public:
    std::shared_ptr<SimulationManagerAccessorBase> manager; //!< SimulationManager の Accessor
    bool _removeWhenKilled; //!< kill されたときに削除するかどうかを表す内部変数。デフォルトはfalse。ただし、自身がcomponentの場合はholderに連動する。
    std::map<std::string,std::weak_ptr<CommunicationBuffer>> communicationBuffers; //!< 自身が加入している CommunicationBuffer
    nl::json observables; //!< 他者が観測してよい自身の情報
    nl::json commands; //!< 自身の行動を制御するための情報
    bool _isAlive; //!< 生存状態を表す内部変数。デフォルトはtrue。
    bool _isActive; //!< 活性化されているかどうかを表す内部変数。デフォルトはtrue。modelConfig又はinstanceConfigの"_isActive"にfalseをセットすることで非活性状態で生成できる。
    bool _hasBeenActivated; //!< 活性化されたことがあるかどうかを返す内部変数。現在も活性状態であるかは問わない。
    Time latestActivationTime; //!< 直近の活性化時刻。
    //constructors & destructor
    Asset(const nl::json& modelConfig_,const nl::json& instanceConfig_);
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual bool isAlive() const; //!< 自身が生存しているかどうかを返す。デフォルトの生存条件は_isAliveがtrueかつ、自身がcomponentの場合にholderがaliveであること。
    virtual bool removeWhenKilled() const; //!< kill されたときに削除するかどうかを返す。デフォルトの挙動は、自身がcomponentの場合はholder(isManaged())->removeWhenKilled()に連動、componentでない場合は_removeWhenKilledの値。
    virtual void activate(bool onInitialize=false); //!< 自身を活性化する処理を記述する。デフォルトの挙動は_isActiveをtrueにするのみ。
    virtual void deactivate(bool onInitialize=false); //!< 自身を活性化する処理を記述する。デフォルトの挙動は_isActiveをfalseにするのみ。
    virtual bool isActive() const; //!< 自身が活性状態かどうかを返す処理を記述する。デフォルトの挙動は_isActiveがtrueかどうかで判定する。
    virtual bool hasBeenActivated() const; //!< 活性化されたことがあるかどうかを返す。現在も活性状態であるかは問わない。
    virtual double getTimeSinceLatestActivation() const; //!< 直近の活性化からの経過時間[s]を返す。一度も活性化されていない場合は-1を返す。
    virtual std::string getTeam() const=0; //!< 自身の陣営名(full nameを"/"で区切った先頭要素。"/"が無い場合は空)を返す。
    virtual std::string getGroup() const=0; //!< 自身のグループ名(full nameを"/"で区切った先頭と末尾以外の要素。"/"が二つ以上無い場合は空)を返す。
    virtual std::string getName() const=0; //!< 自身のfull nameを"/"で区切った末尾の要素を返す。
    virtual void addCommunicationBuffer(const std::shared_ptr<CommunicationBuffer>& buffer); //!< [For internal use] 自身が加入した CommunicationBuffer インスタンスを受け取り、メンバに格納する。
    virtual void perceive(bool inReset); //!< 観測に関する周期処理を記述する。引数は SimulationManager::reset 中で呼ばれた場合のみtrueとなる。
    virtual void control(); //!< 制御に関する周期処理を記述する。
    virtual void behave(); //!< 行動に関する周期処理を記述する。
    virtual void kill(); //!< 自身を非生存状態にする処理を記述する。
    virtual std::shared_ptr<EntityAccessor> getAccessorImpl() override;
    virtual std::set<std::string> getUnmanagedComponentPhases() override;
    protected:
    virtual void setManagerAccessor(const std::shared_ptr<EntityManagerAccessor>& ema) override;
};

/**
 * @class AssetAccessor
 * @brief Asset とその派生クラスのメンバに対するアクセス制限を実現するためのAccessorクラス。
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(AssetAccessor,EntityAccessor)
    public:
    AssetAccessor(const std::shared_ptr<Asset>& a);
    virtual bool isAlive() const; //!< 自身が生存しているかどうかを返す。デフォルトの生存条件は_isAliveがtrueかつ、自身がcomponentの場合にholderがaliveであること。
    virtual bool removeWhenKilled() const; //!< kill されたときに削除するかどうかを返す。デフォルトの挙動は、自身がcomponentの場合はholder(isManaged())->removeWhenKilled()に連動、componentでない場合は_removeWhenKilledの値。
    virtual bool isActive() const; //!< 自身が活性状態かどうかを返す処理を記述する。デフォルトの挙動は_isActiveがtrueかどうかで判定する。
    virtual bool hasBeenActivated() const; //!< 活性化されたことがあるかどうかを返す。現在も活性状態であるかは問わない。
    virtual double getTimeSinceLatestActivation() const; //!< 直近の活性化からの経過時間[s]を返す。一度も活性化されていない場合は-1を返す。
    virtual std::string getTeam() const; //!< 自身の陣営名(full nameを"/"で区切った先頭要素。"/"が無い場合は空)を返す。
    virtual std::string getGroup() const; //!< 自身のグループ名(full nameを"/"で区切った先頭と末尾以外の要素。"/"が二つ以上無い場合は空)を返す。
    virtual std::string getName() const; //!< 自身のfull nameを"/"で区切った末尾の要素を返す。
    const nl::json& observables; //!< 自身のobservablesのconst参照。Python側からもread-onlyプロパティとして参照可能
    protected:
    static nl::json dummy;
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(Asset)
    virtual bool isAlive() const override{
        PYBIND11_OVERRIDE(bool,Base,isAlive);
    }
    virtual bool removeWhenKilled() const override{
        PYBIND11_OVERRIDE(bool,Base,removeWhenKilled);
    }
    virtual void activate(bool onInitialize=false)  override{
        PYBIND11_OVERRIDE(void,Base,activate,onInitialize);
    }
    virtual void deactivate(bool onInitialize=false) override{
        PYBIND11_OVERRIDE(void,Base,deactivate,onInitialize);
    }
    virtual bool isActive() const override{
        PYBIND11_OVERRIDE(bool,Base,isActive);
    }
    virtual bool hasBeenActivated() const override{
        PYBIND11_OVERRIDE(bool,Base,hasBeenActivated);
    }
    virtual double getTimeSinceLatestActivation() const override{
        PYBIND11_OVERRIDE(double,Base,getTimeSinceLatestActivation);
    }
    virtual std::string getTeam() const override{
        PYBIND11_OVERRIDE_PURE(std::string,Base,getTeam);
    }
    virtual std::string getGroup() const override{
        PYBIND11_OVERRIDE_PURE(std::string,Base,getGroup);
    }
    virtual std::string getName() const override{
        PYBIND11_OVERRIDE_PURE(std::string,Base,getName);
    }
    virtual void addCommunicationBuffer(const std::shared_ptr<CommunicationBuffer>& buffer) override{
        PYBIND11_OVERRIDE(void,Base,addCommunicationBuffer,buffer);
    }
    virtual void perceive(bool inReset) override{
        PYBIND11_OVERRIDE(void,Base,perceive,inReset);
    }
    virtual void control() override{
        PYBIND11_OVERRIDE(void,Base,control);
    }
    virtual void behave() override{
        PYBIND11_OVERRIDE(void,Base,behave);
    }
    virtual void kill() override{
        PYBIND11_OVERRIDE(void,Base,kill);
    }
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(AssetAccessor)
    virtual bool isAlive() const override{
        PYBIND11_OVERRIDE(bool,Base,isAlive);
    }
    virtual bool removeWhenKilled() const override{
        PYBIND11_OVERRIDE(bool,Base,removeWhenKilled);
    }
    virtual bool isActive() const override{
        PYBIND11_OVERRIDE(bool,Base,isActive);
    }
    virtual bool hasBeenActivated() const override{
        PYBIND11_OVERRIDE(bool,Base,hasBeenActivated);
    }
    virtual double getTimeSinceLatestActivation() const override{
        PYBIND11_OVERRIDE(double,Base,getTimeSinceLatestActivation);
    }
    virtual std::string getTeam() const override{
        PYBIND11_OVERRIDE(std::string,Base,getTeam);
    }
    virtual std::string getGroup() const override{
        PYBIND11_OVERRIDE(std::string,Base,getGroup);
    }
    virtual std::string getName() const override{
        PYBIND11_OVERRIDE(std::string,Base,getName);
    }
};

void exportAsset(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
