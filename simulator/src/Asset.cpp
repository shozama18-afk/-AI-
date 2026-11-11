// Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "Asset.h"
#include "Utility.h"
#include "SimulationManager.h"
#include "Agent.h"
#include "CommunicationBuffer.h"
ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
using namespace util;

Asset::Asset(const nl::json& modelConfig_,const nl::json& instanceConfig_)
:StructuredEntity(modelConfig_,instanceConfig_),observables(nl::json::object()),commands(nl::json::object()){
}
void Asset::initialize(){
    BaseType::initialize();

    _isAlive=true;

    if(instanceConfig.contains("removeWhenKilled")){
        _removeWhenKilled=getValueFromJsonK(instanceConfig,"removeWhenKilled");
    }else if(modelConfig.contains("removeWhenKilled")){
        _removeWhenKilled=getValueFromJsonK(modelConfig,"removeWhenKilled");
    }else{
        _removeWhenKilled=false;
    }

    _isActive=false;
    _hasBeenActivated=false;
    latestActivationTime=Time();
    bool _isActiveOnInitialize=true;
    if(instanceConfig.contains("isActive")){
        _isActiveOnInitialize=getValueFromJsonK(instanceConfig,"isActive");
    }else if(modelConfig.contains("isActive")){
        _isActiveOnInitialize=getValueFromJsonK(modelConfig,"isActive");
    }
    if(_isActiveOnInitialize){
        activate(true);
    }

    std::shared_ptr<SimulationManagerAccessorBase> manager=std::dynamic_pointer_cast<SimulationManagerAccessorBase>(BaseType::manager);
    if(modelConfig.contains("interval")){
        nl::json intervals=modelConfig.at("interval");
        std::string intervalUnit="tick";
        if(intervals.contains("unit")){
            intervalUnit=intervals.at("unit");
        }
        if(intervalUnit=="time"){
            double dt=manager->getBaseTimeStep();
            interval[SimPhase::PERCEIVE]=std::max<std::uint64_t>(1,std::lround(getValueFromJsonKRD(intervals,"perceive",randomGen,dt)/dt));//reset at t=0 is not regarded as the "first time."
            interval[SimPhase::CONTROL]=std::max<std::uint64_t>(1,std::lround(getValueFromJsonKRD(intervals,"control",randomGen,dt)/dt));
            interval[SimPhase::BEHAVE]=std::max<std::uint64_t>(1,std::lround(getValueFromJsonKRD(intervals,"behave",randomGen,dt)/dt));
        }else if(intervalUnit=="tick"){
            interval[SimPhase::PERCEIVE]=getValueFromJsonKRD(intervals,"perceive",randomGen,1);//reset at t=0 is not regarded as the "first time."
            interval[SimPhase::CONTROL]=getValueFromJsonKRD(intervals,"control",randomGen,1);
            interval[SimPhase::BEHAVE]=getValueFromJsonKRD(intervals,"behave",randomGen,1);
        }else{
            throw std::runtime_error("Only \"tick\" or \"time\" can be designated as the unit of interval."+intervalUnit);
        }
    }else{
        interval[SimPhase::PERCEIVE]=1;
        interval[SimPhase::CONTROL]=1;
        interval[SimPhase::BEHAVE]=1;
    }
    interval[SimPhase::VALIDATE]=std::numeric_limits<std::uint64_t>::max();
    if(modelConfig.contains("firstTick")){
        nl::json firstTicks=modelConfig.at("firstTick");
        std::string firstTickUnit="tick";
        if(firstTicks.contains("unit")){
            firstTickUnit=firstTicks.at("unit");
        }
        if(firstTickUnit=="time"){
            double dt=manager->getBaseTimeStep();
            firstTick[SimPhase::PERCEIVE]=std::lround(getValueFromJsonKRD<double>(firstTicks,"perceive",randomGen,interval[SimPhase::PERCEIVE]*dt)/dt);//reset at t=0 is not regarded as the "first time."
            firstTick[SimPhase::CONTROL]=std::lround(getValueFromJsonKRD(firstTicks,"control",randomGen,0.0)/dt);
            firstTick[SimPhase::BEHAVE]=std::lround(getValueFromJsonKRD(firstTicks,"behave",randomGen,0.0)/dt);
        }else if(firstTickUnit=="tick"){
            firstTick[SimPhase::PERCEIVE]=getValueFromJsonKRD(firstTicks,"perceive",randomGen,interval[SimPhase::PERCEIVE]);//reset at t=0 is not regarded as the "first time."
            firstTick[SimPhase::CONTROL]=getValueFromJsonKRD(firstTicks,"control",randomGen,0);
            firstTick[SimPhase::BEHAVE]=getValueFromJsonKRD(firstTicks,"behave",randomGen,0);
        }else{
            throw std::runtime_error("Only \"tick\" or \"time\" can be designated as the unit of firstTick."+firstTickUnit);
        }
    }else{
        firstTick[SimPhase::PERCEIVE]=interval[SimPhase::PERCEIVE];//reset at t=0 is not regarded as the "first time."
        firstTick[SimPhase::CONTROL]=0;
        firstTick[SimPhase::BEHAVE]=0;
    }
    firstTick[SimPhase::VALIDATE]=0;//meaningless but defined for same interface
    observables={
        {"fullName",getFullName()},
        {"isAlive",isAlive()},
        {"removeWhenKilled",removeWhenKilled()},
        {"isActive",isActive()},
        {"hasBeenActivated",hasBeenActivated()},
        {"timeSinceLatestActivation",getTimeSinceLatestActivation()}
    };
}

void Asset::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    ASRC_SERIALIZE_NVP(archive
        ,observables
        ,commands
        ,_isAlive
        ,_removeWhenKilled
        ,_isActive
        ,_hasBeenActivated
        ,latestActivationTime
    )
}

bool Asset::isAlive() const{
    if(isComponent(isManaged())){
        // 他のAssetのcomponentのとき、holderがaliveでなければ自身も連動して非alive
        if(isHolderExpired(isManaged()) || !holder<Asset>(isManaged())->isAlive()){
            return false;
        }
    }
    // 他のAssetによって決まらない場合は、自身固有のフラグで判定する。
    return _isAlive;
}

bool Asset::removeWhenKilled() const{
    if(isComponent(isManaged())){
        // 他のAssetのcomponentのとき、holderがkill時にremoveされるなら連動してremove
        if(isHolderExpired(isManaged()) || holder<Asset>(isManaged())->removeWhenKilled()){
            return true;
        }
    }
    // 他のAssetによって決まらない場合は、自身固有のフラグで判定する。
    return _removeWhenKilled;
}

void Asset::activate(bool onInitialize){
    if(onInitialize || !_isActive){
        _isActive=true;
        _hasBeenActivated=true;
        latestActivationTime=manager->getTime();
    }
}

void Asset::deactivate(bool onInitialize){
    if(onInitialize || _isActive){
        _isActive=false;
    }
}

bool Asset::isActive() const{
    return _isActive;
}

bool Asset::hasBeenActivated() const{
    return _hasBeenActivated;
}

double Asset::getTimeSinceLatestActivation() const{
    if(hasBeenActivated()){
        return manager->getTime()-latestActivationTime;
    }else{
        return -1;
    }
}

void Asset::addCommunicationBuffer(const std::shared_ptr<CommunicationBuffer>& buffer){
    communicationBuffers[buffer->getName()]=buffer;
}

void Asset::perceive(bool inReset){
    observables["fullName"]=getFullName();
    observables["isAlive"]=isAlive();
    observables["removeWhenKilled"]=removeWhenKilled();
    observables["isActive"]=isActive();
    observables["hasBeenActivated"]=hasBeenActivated();
    observables["timeSinceLatestActivation"]=getTimeSinceLatestActivation();
}
void Asset::control(){
    if(isActive()){
        if(commands.contains("deactivate") && commands.at("deactivate")){
            deactivate(false);
        }
    }else{
        if(commands.contains("activate") && commands.at("activate")){
            activate(false);
        }
    }
    commands.merge_patch({
        {"activate",nullptr},
        {"deactivate",nullptr}
    });
}
void Asset::behave(){
    if(isActive()){
        if(commands.contains("deactivate") && commands.at("deactivate")){
            deactivate(false);
        }
    }else{
        if(commands.contains("activate") && commands.at("activate")){
            activate(false);
        }
    }
    commands.merge_patch({
        {"activate",nullptr},
        {"deactivate",nullptr}
    });
}
void Asset::kill(){
    if(isActive()){
        deactivate(false);
    }
    _isAlive=false;
    if(isManaged()){
        for(auto&& component : getManagedComponents<Asset>()){
            if(component){
                component->kill();
            }
        }
    }
    for(auto&& component : getUnmanagedComponents<Asset>()){
        if(component){
            component->kill();
        }
    }

    observables={
        {"fullName",getFullName()},
        {"isAlive",false}
    };
    commands=nl::json::object();
}
std::shared_ptr<EntityAccessor> Asset::getAccessorImpl(){
    return std::make_shared<AssetAccessor>(getShared<Asset>(this->shared_from_this()));
}

std::set<std::string> Asset::getUnmanagedComponentPhases(){
    return {
        enumToStr(SimPhase::VALIDATE),
        enumToStr(SimPhase::PERCEIVE),
        enumToStr(SimPhase::CONTROL),
        enumToStr(SimPhase::BEHAVE)
    };
}

void Asset::setManagerAccessor(const std::shared_ptr<EntityManagerAccessor>& ema){
    BaseType::setManagerAccessor(ema);
    manager=std::dynamic_pointer_cast<SimulationManagerAccessorBase>(ema);
    assert(manager);
}

nl::json AssetAccessor::dummy=nl::json();
AssetAccessor::AssetAccessor(const std::shared_ptr<Asset>& a)
:EntityAccessor(a)
,observables(a ? a->observables : dummy)
{
}
bool AssetAccessor::isAlive() const{
    auto asset=get<Asset>(false);
    return asset && asset->isAlive();
}
bool AssetAccessor::removeWhenKilled() const{
    return get<Asset>(true)->removeWhenKilled();
}
bool AssetAccessor::isActive() const{
    auto asset=get<Asset>(false);
    return asset && asset->isActive();
}
bool AssetAccessor::hasBeenActivated() const{
    return get<Asset>(true)->hasBeenActivated();
}
double AssetAccessor::getTimeSinceLatestActivation() const{
    return get<Asset>(true)->getTimeSinceLatestActivation();
}
std::string AssetAccessor::getTeam() const{
    return get<Asset>(true)->getTeam();
}
std::string AssetAccessor::getGroup() const{
    return get<Asset>(true)->getGroup();
}
std::string AssetAccessor::getName() const{
    return get<Asset>(true)->getName();
}

void exportAsset(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;
    expose_common_class<Asset>(m,"Asset")
    DEF_FUNC(Asset,isAlive)
    DEF_FUNC(Asset,removeWhenKilled)
    DEF_FUNC(Asset,activate)
    DEF_FUNC(Asset,deactivate)
    DEF_FUNC(Asset,isActive)
    DEF_FUNC(Asset,hasBeenActivated)
    DEF_FUNC(Asset,getTimeSinceLatestActivation)
    DEF_FUNC(Asset,getTeam)
    DEF_FUNC(Asset,getGroup)
    DEF_FUNC(Asset,getName)
    DEF_FUNC(Asset,perceive)
    DEF_FUNC(Asset,control)
    DEF_FUNC(Asset,behave)
    DEF_FUNC(Asset,kill)
    DEF_READWRITE(Asset,_isAlive)
    DEF_READWRITE(Asset,_removeWhenKilled)
    DEF_READWRITE(Asset,_isActive)
    DEF_READWRITE(Asset,_hasBeenActivated)
    DEF_READWRITE(Asset,latestActivationTime)
    DEF_READWRITE(Asset,communicationBuffers)
    DEF_READWRITE(Asset,observables)
    DEF_READWRITE(Asset,commands)
    ;

    expose_common_class<AssetAccessor>(m,"AssetAccessor")
    .def(py_init<const std::shared_ptr<Asset>&>())
    DEF_FUNC(AssetAccessor,isAlive)
    DEF_FUNC(AssetAccessor,removeWhenKilled)
    DEF_FUNC(AssetAccessor,isActive)
    DEF_FUNC(AssetAccessor,hasBeenActivated)
    DEF_FUNC(AssetAccessor,getTimeSinceLatestActivation)
    DEF_FUNC(AssetAccessor,getTeam)
    DEF_FUNC(AssetAccessor,getGroup)
    DEF_FUNC(AssetAccessor,getName)
    .def_property_readonly("observables",[](const AssetAccessor& v){return v.observables;})
    ;
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
