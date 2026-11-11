// Copyright (c) 2024-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
/**
 * @file
 */
#include "StructuredEntity.h"
#include "Utility.h"
#include "Factory.h"
#include "SimulationManager.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
using namespace util;

void StructuredEntity::initialize(){
    BaseType::initialize();

    if(instanceConfig.contains("holder")){
        instanceConfig.at("holder").get_to(_holder);
        assert(!_holder.expired());
        _isComponent=true;
    }else{
        _isComponent=false;
    }

    if(_isComponent){
        if(instanceConfig.contains("root")){
            instanceConfig.at("root").get_to(_root);
            assert(!_root.expired());
        }else{
            throw std::runtime_error("Non-root StructuredEntity requires 'root' StructuredEntity in the instanceConfig.");
        }
    }

    registerComponent(false,root(false),non_const_this());
    if(isManaged()){
        registerComponent(true,root(true),non_const_this());
    }
}

void StructuredEntity::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    // setDependencyはSimulationManagerによって呼ばれる。

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,_isComponent
            ,_root
            ,_holder
        )
    }

    if(isManaged() && isRoot(true)){
        ASRC_SERIALIZE_NVP(archive
            ,_managedComponents
        )
    }
    if(isRoot(false)){
        ASRC_SERIALIZE_NVP(archive
            ,_unmanagedComponents
            ,_unmanagedComponentDependencyGraph
            ,_updateCountOfUnmanagedComponentDependencyGraph
        )
    }
}

bool StructuredEntity::isRoot(bool inManagedTree) const{
    if(isManaged() && !inManagedTree){
        // managedはつねにunmanaged treeのrootとしても振る舞う
        return true;
    }else if(inManagedTree && !isManaged()){
        // unmanagedはmanaged treeに属さないので例外を投げる
        throw std::runtime_error("StructuredEntity::isRoot(true) failed. Unmanaged StructuredEntity cannot belong to a managed component tree.");
    }
    return !_isComponent;
}

bool StructuredEntity::isComponent(bool inManagedTree) const{
    if(isManaged() && !inManagedTree){
        // managedはつねにunmanaged treeのrootとしても振る舞う
        return false;
    }else if(inManagedTree && !isManaged()){
        // unmanagedはmanaged treeに属さないので例外を投げる
        throw std::runtime_error("StructuredEntity::isComponent(true) failed. Unmanaged StructuredEntity cannot belong to a managed component tree.");
    }
    return _isComponent;
}

std::string StructuredEntity::getComponentPath(bool inManagedTree)  const{
    if(inManagedTree){
        return "/"+getManagedComponentTree(root(inManagedTree)).get_path(non_const_this());
    }else{
        return "/"+getUnmanagedComponentTree(root(inManagedTree)).get_path(non_const_this());
    }
}

std::string StructuredEntity::getComponentName(bool inManagedTree)  const{
    if(inManagedTree){
        return getManagedComponentTree(root(inManagedTree)).get_name(non_const_this());
    }else{
        return getUnmanagedComponentTree(root(inManagedTree)).get_name(non_const_this());
    }
}

bool StructuredEntity::isRootExpired(bool inManagedTree) const{
    if(isRoot(inManagedTree)){
        return false;
    }else{
        if(inManagedTree && !isManaged()){
            throw std::runtime_error("StructuredEntity::isRootExpired(true) failed. Unmanaged StructuredEntity cannot belong to a managed tree.");
        }
        return _root.expired();
    }
}

bool StructuredEntity::isHolderExpired(bool inManagedTree) const{
    if(isRoot(inManagedTree)){
        return true;
    }else{
        if(inManagedTree && !isManaged()){
            throw std::runtime_error("StructuredEntity::isHolderExpired(true) failed. Unmanaged StructuredEntity cannot belong to a managed tree.");
        }
        return _holder.expired();
    }
}

void StructuredEntity::setDependency(){
    //Inform children parent's sequential process order.
    //Call addDependency(phase,predecessor,successor) to add dependency.
}

void StructuredEntity::addDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(predecessor->isManaged() && successor->isManaged()){
        addManagedDependency(phase,predecessor,successor);
    }else{
        if(predecessor->root(false)==successor->root(false)){
            addUnmanagedDependency(predecessor->root(false),enumToStr(phase),predecessor,successor);
        }else{
            throw std::runtime_error("not on the same unmanaged component tree");
        }
    }
}

void StructuredEntity::addDependency(const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(predecessor->isManaged() && successor->isManaged()){
        std::string upper=phase;
        std::transform(
            phase.begin(),
            phase.end(),
            upper.begin(),
            [](char c){return std::toupper(c);}
        );
        SimPhase asEnum;
        try{
            asEnum=strToEnum<SimPhase>(upper);
        }catch(...){
            throw std::runtime_error("addDependency between managed components requires phase to be a value of SimPhase, but '"+upper+"' is not a value of SimPhase.");
        }
        addManagedDependency(asEnum,predecessor,successor);
    }else{
        if(predecessor->root(false)==successor->root(false)){
            addUnmanagedDependency(predecessor->root(false),phase,predecessor,successor);
        }else{
            throw std::runtime_error("not on the same unmanaged component tree");
        }
    }
}

void StructuredEntity::removeDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(predecessor->isManaged() && successor->isManaged()){
        removeManagedDependency(phase,predecessor,successor);
    }else{
        if(predecessor->root(false)==successor->root(false)){
            removeUnmanagedDependency(predecessor->root(false),enumToStr(phase),predecessor,successor);
        }else{
            throw std::runtime_error("not on the same unmanaged component tree");
        }
    }
}

void StructuredEntity::removeDependency(const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    if(predecessor->isManaged() && successor->isManaged()){
        std::string upper=phase;
        std::transform(
            phase.begin(),
            phase.end(),
            upper.begin(),
            [](char c){return std::toupper(c);}
        );
        SimPhase asEnum;
        try{
            asEnum=strToEnum<SimPhase>(phase);
        }catch(...){
            throw std::runtime_error("removeDependency between managed components requires phase to be a value of SimPhase, but '"+upper+"' is not a value of SimPhase.");
        }
        removeManagedDependency(asEnum,predecessor,successor);
    }else{
        if(predecessor->root(false)==successor->root(false)){
            removeUnmanagedDependency(predecessor->root(false),phase,predecessor,successor);
        }else{
            throw std::runtime_error("not on the same unmanaged component tree");
        }
    }
}

bool StructuredEntity::hasComponent(bool inManagedTree,const std::string& path) const{
    if(inManagedTree && !isManaged()){
        throw std::runtime_error("Only managed Entity can search a component in the managed tree.");
    }
    std::string absPath=path;
    if(absPath.size()==0 || absPath.at(0)!='/'){
        // 相対パス→絶対パスに変換
        absPath=getComponentPath(inManagedTree)+"/"+absPath;
    }
    std::shared_ptr<StructuredEntity> component;
    if(inManagedTree){
        return getManagedComponentTree(root(inManagedTree)).contains(absPath);
    }else{
        return getUnmanagedComponentTree(root(inManagedTree)).contains(absPath);
    }
}

std::set<std::string> StructuredEntity::getUnmanagedComponentPhases(){
    return {};
}

nl::json StructuredEntity::getNewComponentConfig(bool isManaged_, const std::string& newComponentName, const nl::json& newComponentConfig){
    std::string baseName=newComponentConfig.at("baseName");

    nl::json ic = newComponentConfig.contains("instanceConfig") ? newComponentConfig.at("instanceConfig") : nl::json::object();
    ic["entityFullName"]=getFullName()+"/"+newComponentName;
    ic["seed"]=randomGen();
    ic["root"]=root(isManaged_);
    ic["holder"]=non_const_this();

    if(baseName=="PlaceHolder"){
        return {
            {"isEpisodic",isEpisodic()},
            {"baseName",baseName},
            {"className","PlaceHolderEntity"},
            {"modelName",""},
            {"modelConfig",nl::json::object()},
            {"instanceConfig",std::move(ic)}
        };
    }

    auto [cn,mn,mc,chain] = resolveModelConfig(newComponentConfig);

    if(isManaged_){
        // managedの場合、firstTick,intervalが未設定なら生成者(holder)と同じとする。
        if(!mc.contains("firstTick") && !ic.contains("firstTick")){
            if(modelConfig.contains("firstTick")){
                mc["firstTick"]=modelConfig.at("firstTick");
            }
        }
        if(!mc.contains("interval") && !ic.contains("interval")){
            if(modelConfig.contains("interval")){
                mc["interval"]=modelConfig.at("interval");
            }
        }
    }else{
        // unmanagedならつねに生成者(holder)と同じfirstTick,intervalを設定する。
        if(modelConfig.contains("firstTick")){
            mc["firstTick"]=modelConfig.at("firstTick");
        }else{
            if(mc.contains("firstTick")){
                mc.erase("firstTick");
            }
        }
        if(modelConfig.contains("interval")){
            mc["interval"]=modelConfig.at("interval");
        }else{
            if(mc.contains("interval")){
                mc.erase("interval");
            }
        }
    }

    return {
        {"isEpisodic",isEpisodic()},
        {"baseName",baseName},
        {"className",cn},
        {"modelName",mn},
        {"modelConfig",std::move(mc)},
        {"instanceConfig",std::move(ic)}
    };
}

void StructuredEntity::addManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    throw std::runtime_error("Only Callback, Controller, or PhysicalAsset can add dependency between managed Entities.");
}

void StructuredEntity::removeManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    throw std::runtime_error("Only Callback, Controller, or PhysicalAsset can remove dependency between managed Entities.");
}

EntityComponentTree<StructuredEntity,true>& StructuredEntity::getManagedComponentTree(const std::shared_ptr<StructuredEntity>& root){
    return root->_managedComponents;
}

EntityComponentTree<StructuredEntity,false>& StructuredEntity::getUnmanagedComponentTree(const std::shared_ptr<StructuredEntity>& root){
    return root->_unmanagedComponents;
}

EntityDependencyGraph<StructuredEntity>& StructuredEntity::getUnmanagedComponentDependencyGraph(const std::shared_ptr<StructuredEntity>& root,const std::string& phase){
    return root->_unmanagedComponentDependencyGraph[phase];
}

void StructuredEntity::registerComponent(bool inManagedTree,const std::shared_ptr<StructuredEntity>& root, const std::shared_ptr<StructuredEntity>& component){
    std::string path=std::filesystem::path(component->getFullName()).lexically_relative(root->getFullName()).generic_string();
    if(inManagedTree){
        assert(component->isManaged());
        root->_managedComponents.put(path,component);
    }else{
        assert(root==component || !component->isManaged());
        root->_unmanagedComponents.put(path,component);
        for(auto&& phase : root->getUnmanagedComponentPhases()){
            root->_unmanagedComponentDependencyGraph[phase].add_instance(component);
        }
    }
}

void StructuredEntity::addUnmanagedDependency(const std::shared_ptr<StructuredEntity>& root, const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    assert(root->_unmanagedComponentDependencyGraph[phase].contains(predecessor));
    assert(root->_unmanagedComponentDependencyGraph[phase].contains(successor));
    bool updated=root->_unmanagedComponentDependencyGraph[phase].add_dependency(predecessor,successor);
    if(updated){
        root->_updateCountOfUnmanagedComponentDependencyGraph[phase]++;
    }
}

void StructuredEntity::removeUnmanagedDependency(const std::shared_ptr<StructuredEntity>& root, const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor){
    assert(root->_unmanagedComponentDependencyGraph[phase].contains(predecessor));
    assert(root->_unmanagedComponentDependencyGraph[phase].contains(successor));
    root->_unmanagedComponentDependencyGraph[phase].remove_dependency(predecessor,successor);
}

const std::string PlaceHolderEntity::baseName="PlaceHolder"; // baseNameを上書き

void exportStructuredEntity(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;

    expose_component_tree_name<EntityComponentTree<StructuredEntity>>(m,"StructuredEntityComponentTree");
    expose_component_tree_name<EntityComponentTree<StructuredEntity,true>>(m,"WeakStructuredEntityComponentTree");

    expose_dependency_graph_name<EntityDependencyGraph<StructuredEntity>>(m,"StructuredEntityDependencyGraph");

    using ManagedComponentsRangeType = decltype(std::declval<StructuredEntity>().getManagedComponents<>());
	expose_range<ManagedComponentsRangeType>(m);
    using UnmanagedComponentsRangeType1 = decltype(std::declval<StructuredEntity>().getUnmanagedComponents<>());
	expose_range<UnmanagedComponentsRangeType1>(m);
    using UnmanagedComponentsRangeType2 = decltype(std::declval<StructuredEntity>().getUnmanagedComponents<>(""));
	expose_range<UnmanagedComponentsRangeType2>(m);

    // StructuredEntityは純粋仮想関数を持っており、それはC++側の派生クラス側で定義する前提である。
    // そのため、StructuredEntityを直接Python側から継承したりインスタンス化したりすることはない。
    // よって、py_initが不要となるためexpose_entity_subclassではなくexpose_common_classを用いる。
    expose_common_class<StructuredEntity>(m,"StructuredEntity")
    ASRC_DEF_FUNC(StructuredEntity,isRoot)
    ASRC_DEF_FUNC(StructuredEntity,isComponent)
    ASRC_DEF_FUNC(StructuredEntity,getComponentPath)
    ASRC_DEF_FUNC(StructuredEntity,isRootExpired)
    ASRC_DEF_FUNC(StructuredEntity,isHolderExpired)
    .def("root",&StructuredEntity::root<>)
    .def("holder",&StructuredEntity::root<>)
    .def("createComponent",&StructuredEntity::createComponent<>)
    ASRC_DEF_FUNC(StructuredEntity,setDependency)
    .def("addDependency",py::overload_cast<const SimPhase&,const std::shared_ptr<StructuredEntity>&,const std::shared_ptr<StructuredEntity>&>(&StructuredEntity::addDependency))
    .def("addDependency",py::overload_cast<const std::string&,const std::shared_ptr<StructuredEntity>&,const std::shared_ptr<StructuredEntity>&>(&StructuredEntity::addDependency))
    .def("removeDependency",py::overload_cast<const SimPhase&,const std::shared_ptr<StructuredEntity>&,const std::shared_ptr<StructuredEntity>&>(&StructuredEntity::removeDependency))
    .def("removeDependency",py::overload_cast<const std::string&,const std::shared_ptr<StructuredEntity>&,const std::shared_ptr<StructuredEntity>&>(&StructuredEntity::removeDependency))
    .def("getComponent",&StructuredEntity::getComponent<>)
    .def("getManagedComponents",&StructuredEntity::getManagedComponents<>,pybind11::return_value_policy::reference_internal)
    .def("getUnmanagedComponents",py::overload_cast<>(&StructuredEntity::getUnmanagedComponents<>,py::const_),pybind11::return_value_policy::reference_internal)
    .def("getUnmanagedComponents",py::overload_cast<const std::string&>(&StructuredEntity::getUnmanagedComponents<>,py::const_),pybind11::return_value_policy::reference_internal)
    ASRC_DEF_FUNC(StructuredEntity,getAllUnmanagedComponentsInTree<>)
    ASRC_DEF_FUNC(StructuredEntity,getUnmanagedComponentPhases)
    ;

    expose_common_class<PlaceHolderEntity>(m,"PlaceHolderEntity")
    ;
    FACTORY_ADD_BASE(PlaceHolderEntity,{"AnyOthers"},{"AnyOthers"})
    FACTORY_ADD_CLASS(PlaceHolder,PlaceHolderEntity)
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
