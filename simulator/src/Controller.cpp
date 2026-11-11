// Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "Controller.h"
#include <pybind11/stl.h>
#include "Utility.h"
#include "Factory.h"
#include "SimulationManager.h"
#include "Asset.h"
#include "Agent.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
using namespace util;

const std::string Controller::baseName="Controller"; // baseNameを上書き

void Controller::initialize(){
    BaseType::initialize();
    auto fullName=getFullName();
    if(fullName.find("/")>=0){
        team=fullName.substr(0,fullName.find("/"));
        group=fullName.substr(0,fullName.rfind("/"));
        name=fullName.substr(fullName.rfind("/")+1);
    }else{
        team=group=name=fullName;
    }
    if(instanceConfig.contains("parent")){
        parent=instanceConfig.at("parent");
    }
    if(parent.expired()){
        throw std::runtime_error("Controller must have a parent. fullName="+getFullName());
    }
}

void Controller::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        if(asrc::core::util::isInputArchive(archive)){
            auto fullName=getFullName();
            if(fullName.find("/")>=0){
                team=fullName.substr(0,fullName.find("/"));
                group=fullName.substr(0,fullName.rfind("/"));
                name=fullName.substr(fullName.rfind("/")+1);
            }else{
                team=group=name=fullName;
            }
        }

        ASRC_SERIALIZE_NVP(archive
            ,parent
        )
    }
}

std::string Controller::getTeam() const{
    return team;
}
std::string Controller::getGroup() const{
    return group;
}
std::string Controller::getName() const{
    return name;
}
void Controller::perceive(bool inReset){
    BaseType::perceive(inReset);
}
void Controller::control(){
    BaseType::control();
}
void Controller::behave(){
    BaseType::behave();
}

bool Controller::generateCommunicationBuffer(const std::string& name_,const nl::json& participants_,const nl::json& inviteOnRequest_){
    return manager->generateCommunicationBuffer(name_,participants_,inviteOnRequest_);
}
void Controller::setManagerAccessor(const std::shared_ptr<EntityManagerAccessor>& ema){
    BaseType::setManagerAccessor(ema);
    manager=std::dynamic_pointer_cast<SimulationManagerAccessorForPhysicalAsset>(ema);
    assert(manager);
}

nl::json Controller::getNewComponentConfig(bool isManaged_, const std::string& newComponentName, const nl::json& newComponentConfig){
    auto ret=BaseType::getNewComponentConfig(isManaged_, newComponentName, newComponentConfig);
    std::string baseName=ret.at("baseName");
    if(baseName!="Controller"){
        throw std::runtime_error("Controller can only create Controller as a component.");
    }
    return std::move(ret);
}

void Controller::addManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(!isinstance<Asset>(predecessor) || isinstance<Asset>(successor)){
        throw std::runtime_error("Controller can add dependency only between two Assets.");
    }
    return manager->addDependency(phase,getShared<Asset>(predecessor),getShared<Asset>(successor));
}

void Controller::removeManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(!isinstance<Asset>(predecessor) || isinstance<Asset>(successor)){
        throw std::runtime_error("Controller can remove dependency only between two Assets.");
    }
    return manager->removeDependency(phase,getShared<Asset>(predecessor),getShared<Asset>(successor));
}

void exportController(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;

    FACTORY_ADD_BASE(Controller,{"Controller","PhysicalAsset","Callback","!AnyOthers"},{"!Agent","AnyOthers"})

    expose_entity_subclass<Controller>(m,"Controller")
    DEF_READWRITE(Controller,name)
    DEF_READWRITE(Controller,parent)
    ;
    FACTORY_ADD_CLASS(Controller,Controller)
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
