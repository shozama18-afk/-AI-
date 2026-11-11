// Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "Sensor.h"
#include "Utility.h"
#include "Units.h"
#include "SimulationManager.h"
#include "Agent.h"
#include "Fighter.h"
#include "Missile.h"
#include <boost/uuid/nil_generator.hpp>

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
using namespace util;

void Sensor::initialize(){
    BaseType::initialize();
    assert(!parent.expired() && isBoundToParent);
}
std::pair<bool,std::vector<Track3D>> Sensor3D::isTrackingAny(){
    return std::make_pair(false,std::vector<Track3D>());
}
std::pair<bool,Track3D> Sensor3D::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    return std::make_pair(false,Track3D());
}
std::pair<bool,Track3D> Sensor3D::isTracking(const Track3D& target_){
    return std::make_pair(false,Track3D());
}
std::pair<bool,Track3D> Sensor3D::isTracking(const boost::uuids::uuid& target_){
    return std::make_pair(false,Track3D());
}

std::pair<bool,std::vector<Track2D>> Sensor2D::isTrackingAny(){
    return std::make_pair(false,std::vector<Track2D>());
}
std::pair<bool,Track2D> Sensor2D::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    return std::make_pair(false,Track2D());
}
std::pair<bool,Track2D> Sensor2D::isTracking(const Track2D& target_){
    return std::make_pair(false,Track2D());
}
std::pair<bool,Track2D> Sensor2D::isTracking(const boost::uuids::uuid& target_){
    return std::make_pair(false,Track2D());
}

void AircraftRadar::initialize(){
    BaseType::initialize();
    Lref=getValueFromJsonKR(modelConfig,"Lref",randomGen);
    thetaFOR=deg2rad(getValueFromJsonKR(modelConfig,"thetaFOR",randomGen));
    observables={
        {"spec",{
            {"Lref",Lref},
            {"thetaFOR",thetaFOR}
        }}
    };
}
void AircraftRadar::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,Lref
            ,thetaFOR
        )
    }
    ASRC_SERIALIZE_NVP(archive
        ,track
    )
}
void AircraftRadar::setDependency(){
    //no dependency
}
void AircraftRadar::perceive(bool inReset){
    BaseType::perceive(inReset);
    track.clear();
    if(isActive()){
        auto crs=getParentCRS();// equals to parent.lock()->getBodyCRS() because this is bound to parent.
        if(!crs){
            throw std::runtime_error("crs is nullptr!");
        }
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Fighter>(asset);
        })){
            auto f=getShared<Fighter>(e);
            if(f->isAlive()){
                Eigen::Vector3d fpos=f->pos(crs);
                Eigen::Vector3d rpos=absPtoB(fpos);
                double L=rpos.norm();
                if((thetaFOR<0 || rpos(0)>L*cos(thetaFOR)) && (Lref<0 || L<=Lref*pow(f->rcsScale,0.25))){
                    // Track3D::velはFREE_VECTORなので、目標の速度もFREE_VECTORとしてcrsに変換する。
                    // f->vel(crs)はVELOCITYとして変換され、crs(機体座標系)の運動が差し引かれてしまう。
                    track.push_back(Track3D(f,crs,manager->getTime(),fpos,f->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs)));
                }
            }
        }
    }
    observables["track"]=track;
}
std::pair<bool,std::vector<Track3D>> AircraftRadar::isTrackingAny(){
    if(track.size()>0){
        std::vector<Track3D> ret;
        for(auto& e:track){
            ret.push_back(e);
        }
        return std::make_pair(true,std::move(ret));
    }else{
        return std::make_pair(false,std::vector<Track3D>());
    }
}
std::pair<bool,Track3D> AircraftRadar::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    if(target_.expired()){
        return std::make_pair(false,Track3D());
    }else{
        return isTracking(target_.lock()->getUUIDForTrack());
    }
}
std::pair<bool,Track3D> AircraftRadar::isTracking(const Track3D& target_){
    if(target_.is_none()){
        return std::make_pair(false,Track3D());
    }else{
        return isTracking(target_.truth);
    }
}
std::pair<bool,Track3D> AircraftRadar::isTracking(const boost::uuids::uuid& target_){
    if(target_==boost::uuids::nil_uuid()){
        return std::make_pair(false,Track3D());
    }
    for(auto& e:track){
        if(e.isSame(target_)){
            return std::make_pair(true,e);
        }
    }
    return std::make_pair(false,Track3D());
}

void IRST::initialize(){
    BaseType::initialize();
    Lref=getValueFromJsonKR(modelConfig,"Lref",randomGen);
    thetaFOR=deg2rad(getValueFromJsonKR(modelConfig,"thetaFOR",randomGen));

    track.clear();
    trackUUIDs.clear();

    observables={
        {"spec",{
            {"Lref",Lref},
            {"thetaFOR",thetaFOR}
        }}
    };
}

void IRST::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,Lref
            ,thetaFOR
        )
    }
    ASRC_SERIALIZE_NVP(archive
        ,track
        ,trackUUIDs
    )
}

void IRST::setDependency(){
    //no dependency
}

void IRST::perceive(bool inReset){
    BaseType::perceive(inReset);
    track.clear();

    // 今回の観測ステップで探知できた航跡だけ、前回のUUIDを保持する。
    std::map<boost::uuids::uuid, boost::uuids::uuid> nextTrackUUIDs;

    if(isActive()){
        auto crs=getParentCRS();
        Eigen::Vector3d myPos=pos(crs);
        // Track2D::omegaはFREE_VECTORなので、視線角速度は慣性系での相対速度から求め、crsの軸で表す。
        Eigen::Vector3d myVel=vel.transformTo(manager->getRootCRS()).asTypeOf(CoordinateType::FREE_VECTOR)(crs);
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Fighter>(asset);
        })){
            auto f=getShared<Fighter>(e);
            if(f->isAlive()){
                Eigen::Vector3d fpos=f->pos(crs);
                Eigen::Vector3d rpos=absPtoB(fpos);
                double L=rpos.norm();
                if((thetaFOR<0 || rpos(0)>=L*cos(thetaFOR)) && (Lref<0 || L<=Lref*pow(f->irRadiationIntensityScale,0.5))){
                    // まずは真値を計算
                    Eigen::Vector3d dir=(fpos-myPos)/L;
                    Eigen::Vector3d omega=dir.cross(f->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs)-myVel)/L;

                    // UUIDで味方機の航跡と照合できないようにランダムなUUIDを入れる。
                	// 自機の前ステップの航跡とは照合できるようにする。
                    const auto trueUUID=f->getUUIDForTrack();
                    boost::uuids::uuid anonymousUUID;
                    auto found=trackUUIDs.find(trueUUID);
                    if(found!=trackUUIDs.end()){
                        // 前ステップから連続して探知中
                        anonymousUUID=found->second;
                    }else{
                        // 新規探知または失探後の再探知
                        anonymousUUID=boost::uuids::random_generator()();
                    }
                    nextTrackUUIDs.emplace(
                        trueUUID,
                        anonymousUUID
                    );
                    track.push_back(Track2D(anonymousUUID,crs,manager->getTime(),dir,myPos,omega));
                }
            }
        }
    }

    // 今回探知されなかったFighterの対応はここで破棄される。
    // 次回再探知された場合は新しいUUIDになる。
    trackUUIDs.swap(nextTrackUUIDs);

    observables["track"]=track;
}

std::pair<bool,std::vector<Track2D>> IRST::isTrackingAny(){
    if(track.size()>0){
       std::vector<Track2D> ret;
       for(auto& e:track){
           ret.push_back(e);
       }
        return std::make_pair(true,std::move(ret));
    }else{
        return std::make_pair(false,std::vector<Track2D>());
    }
}

std::pair<bool,Track2D> IRST::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    if(target_.expired()){
        return std::make_pair(false,Track2D());
    }else{
        const auto trueUUID=target_.lock()->getUUIDForTrack();
        auto found=trackUUIDs.find(trueUUID);
        if(found==trackUUIDs.end()){
            return std::make_pair(false,Track2D());
        }
        return isTracking(found->second);
    }
}

std::pair<bool,Track2D> IRST::isTracking(const Track2D& target_){
    if(target_.is_none()){
        return std::make_pair(false,Track2D());
    }else{
        return isTracking(target_.truth);
    }
}

std::pair<bool,Track2D> IRST::isTracking(const boost::uuids::uuid& target_){
    if(target_==boost::uuids::nil_uuid()){
        return std::make_pair(false,Track2D());
    }
    for(auto& e:track){
        if(e.isSame(target_)){
            return std::make_pair(true,e);
        }
    }
    return std::make_pair(false,Track2D());
}

void MWS::initialize(){
    BaseType::initialize();
    isEsmIsh=getValueFromJsonKRD(modelConfig,"isEsmIsh",randomGen,false);
    Lref=getValueFromJsonKRD(modelConfig,"Lref",randomGen,20000.0);
    thetaFOR=deg2rad(getValueFromJsonKRD(modelConfig,"thetaFOR",randomGen,90.0));
    observables={
        {"spec",{
            {"isEsmIsh",isEsmIsh},
            {"Lref",Lref},
            {"thetaFOR",thetaFOR}
        }}
    };
}
void MWS::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,isEsmIsh
            ,Lref
            ,thetaFOR
        )
    }
    ASRC_SERIALIZE_NVP(archive
        ,track
    )
}
void MWS::setDependency(){
    manager->addDependencyGenerator([manager=manager,wPtr=weak_from_this()](const std::shared_ptr<Asset>& asset){
        if(!wPtr.expired()){
            auto ptr=getShared<Type>(wPtr);
            if(
                asset->getTeam()!=ptr->getTeam()
                && isinstance<Missile>(asset)
            ){
                auto m=getShared<const Missile>(asset);
                manager->addDependency(SimPhase::PERCEIVE,m->sensor.lock(),ptr);
            }
        }
    });
}
void MWS::perceive(bool inReset){
    BaseType::perceive(inReset);
    track.clear();
    if(isActive()){
        auto crs=getParentCRS();
        Eigen::Vector3d myPos=pos(crs);
        // Track2D::omegaはFREE_VECTORなので、視線角速度は慣性系での相対速度から求め、crsの軸で表す。
        // Track2D(m,crs,myPos)は観測点を静止点として扱い自機の運動を考慮しないため、ここで明示的に計算する。
        Eigen::Vector3d myVel=vel.transformTo(manager->getRootCRS()).asTypeOf(CoordinateType::FREE_VECTOR)(crs);
        auto makeTrueTrack=[&](const std::shared_ptr<Missile>& m){
            Eigen::Vector3d dr=m->pos(crs)-myPos;
            double R=dr.norm();
            Eigen::Vector3d dir=dr/R;
            Eigen::Vector3d omega=dir.cross(m->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs)-myVel)/R;
            return Track2D(m,crs,manager->getTime(),dir,myPos,omega);
        };
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Missile>(asset);
        })){
            auto m=getShared<Missile>(e);
            Eigen::Vector3d rpos=absPtoB(m->pos(crs));
            double L=rpos.norm();
            if(isEsmIsh){
                if(m->hasLaunched && m->isAlive() && m->sensor.lock()->isActive()){
                    std::pair<bool,Track3D> r=m->sensor.lock()->isTracking(Track3D(parent,crs));
                    if(r.first && (thetaFOR<0 || rpos(0)>=L*cos(thetaFOR)) && (Lref<0 || L<=Lref)){
                        track.push_back(makeTrueTrack(m));//真値を入れる
                    }
                }
            }else{
                if(m->hasLaunched && m->isAlive() && (thetaFOR<0 || rpos(0)>=L*cos(thetaFOR)) && (Lref<0 || L<=Lref)){
                    track.push_back(makeTrueTrack(m));//真値を入れる
                }
            }
        }
    }
    observables["track"]=track;
}
std::pair<bool,std::vector<Track2D>> MWS::isTrackingAny(){
    if(track.size()>0){
       std::vector<Track2D> ret;
       for(auto& e:track){
           ret.push_back(e);
       }
        return std::make_pair(true,std::move(ret));
    }else{
        return std::make_pair(false,std::vector<Track2D>());
    }
}
std::pair<bool,Track2D> MWS::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    if(target_.expired()){
        return std::make_pair(false,Track2D());
    }else{
        return isTracking(target_.lock()->getUUIDForTrack());
    }
}
std::pair<bool,Track2D> MWS::isTracking(const Track2D& target_){
    if(target_.is_none()){
        return std::make_pair(false,Track2D());
    }else{
        return isTracking(target_.truth);
    }
}
std::pair<bool,Track2D> MWS::isTracking(const boost::uuids::uuid& target_){
    if(target_==boost::uuids::nil_uuid()){
        return std::make_pair(false,Track2D());
    }
    for(auto& e:track){
        if(e.isSame(target_)){
            return std::make_pair(true,e);
        }
    }
    return std::make_pair(false,Track2D());
}

void MissileSensor::initialize(){
    BaseType::initialize();
    Lref=getValueFromJsonKR(modelConfig,"Lref",randomGen);
    thetaFOR=deg2rad(getValueFromJsonKR(modelConfig,"thetaFOR",randomGen));
    thetaFOV=deg2rad(getValueFromJsonKR(modelConfig,"thetaFOV",randomGen));
    estTPos=Coordinate(Eigen::Vector3d::Zero(),getParentCRS(),manager->getTime(),CoordinateType::POSITION_ABS);
    observables={
        {"spec",{
            {"Lref",Lref},
            {"thetaFOR",thetaFOR},
            {"thetaFOV",thetaFOV}
        }}
    };
}
void MissileSensor::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,Lref
            ,thetaFOR
            ,thetaFOV
        )
    }
    ASRC_SERIALIZE_NVP(archive
        ,track
        ,target
        ,estTPos
    )
}
void MissileSensor::setDependency(){
    manager->addDependency(SimPhase::CONTROL,parent.lock(),shared_from_this());
}
void MissileSensor::perceive(bool inReset){
    BaseType::perceive(inReset);
    bool isSelfTrackingTarget=isTracking(target).first;
    track.clear();
    if(isActive()){
        auto crs=getParentCRS();
        target=target.extrapolateTo(manager->getTime());
        estTPos=target.pos.transformTo(crs);
        Eigen::Vector3d steer=absPtoB(target.pos(crs)).normalized();
        if(isSelfTrackingTarget){
            for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
                return asset->getTeam()!=getTeam() && isinstance<Fighter>(asset);
            })){
                auto f=getShared<Fighter>(e);
                if(f->isAlive() && target.isSame(f)){
                    steer=absPtoB(f->pos(crs)).normalized();
                    break;
                }
            }
        }
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Fighter>(asset);
        })){
            auto f=getShared<Fighter>(e);
            if(f->isAlive()){
                Eigen::Vector3d fpos=f->pos(crs);
                Eigen::Vector3d rpos=absPtoB(fpos);
                double L=rpos.norm();
                if((thetaFOR<0 || rpos(0)>L*cos(thetaFOR)) && (Lref<0 || L<=Lref) && (thetaFOV<0 || rpos.dot(steer)>L*cos(thetaFOV))){
                    // Track3D::velはFREE_VECTORなので、目標の速度もFREE_VECTORとしてcrsに変換する。
                    track.push_back(Track3D(f,crs,manager->getTime(),fpos,f->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs)));
                }else{
                }
            }
        }
    }
    observables["track"]=track;
}
void MissileSensor::control(){
    BaseType::control();
    nl::json cmd=parent.lock()->commands["Sensor"];
    for(auto&& elem:cmd){
        std::string cmdName=elem.at("name");
        if(cmdName=="steering"){
            auto crs=getParentCRS();
            if(elem.contains("target")){
                target=Track3D(elem.at("target")).extrapolateTo(manager->getTime());
            }else{
                // Old data format
                Coordinate p(elem.at("estTPos"));
                Coordinate v(elem.at("estTVel"));
                target=Track3D(
                    target.truth,
                    p.getCRS(),
                    p.getTime(),
                    p(),
                    v.asTypeOf(CoordinateType::FREE_VECTOR)(p.getCRS())
                ).extrapolateTo(manager->getTime());
            }
            estTPos=target.pos.transformTo(getParentCRS());
        }
    }
}
void MissileSensor::kill(){
    target=Track3D();
    BaseType::kill();
}
std::pair<bool,std::vector<Track3D>> MissileSensor::isTrackingAny(){
    if(track.size()>0){
        std::vector<Track3D> ret;
        for(auto& e:track){
            ret.push_back(e);
        }
        return std::make_pair(true,std::move(ret));
    }else{
        return std::make_pair(false,std::vector<Track3D>());
    }
}
std::pair<bool,Track3D> MissileSensor::isTracking(const std::weak_ptr<PhysicalAsset>& target_){
    if(target_.expired()){
        return std::make_pair(false,Track3D());
    }else{
        return isTracking(target_.lock()->getUUIDForTrack());
    }
}
std::pair<bool,Track3D> MissileSensor::isTracking(const Track3D& target_){
    if(target_.is_none()){
        return std::make_pair(false,Track3D());
    }else{
        return isTracking(target_.truth);
    }
}
std::pair<bool,Track3D> MissileSensor::isTracking(const boost::uuids::uuid& target_){
    if(target_==boost::uuids::nil_uuid()){
        return std::make_pair(false,Track3D());
    }
    if(isActive()){
        for(auto& e:track){
           if(e.isSame(target_)){
               return std::make_pair(true,e);
            }
        }
    }
    return std::make_pair(false,Track3D());
}

void exportSensor(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;
    expose_entity_subclass<Sensor>(m,"Sensor")
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,Sensor)

    expose_entity_subclass<Sensor3D>(m,"Sensor3D")
    DEF_FUNC(Sensor3D,isTrackingAny)
    .def("isTracking",py::overload_cast<const std::weak_ptr<PhysicalAsset>&>(&Sensor3D::isTracking))
    .def("isTracking",py::overload_cast<const Track3D&>(&Sensor3D::isTracking))
    .def("isTracking",py::overload_cast<const boost::uuids::uuid&>(&Sensor3D::isTracking))
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,Sensor3D)

    expose_entity_subclass<Sensor2D>(m,"Sensor2D")
    DEF_FUNC(Sensor2D,isTrackingAny)
    .def("isTracking",py::overload_cast<const std::weak_ptr<PhysicalAsset>&>(&Sensor2D::isTracking))
    .def("isTracking",py::overload_cast<const Track2D&>(&Sensor2D::isTracking))
    .def("isTracking",py::overload_cast<const boost::uuids::uuid&>(&Sensor2D::isTracking))
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,Sensor2D)

    expose_entity_subclass<AircraftRadar>(m,"AircraftRadar")
    DEF_READWRITE(AircraftRadar,Lref)
    DEF_READWRITE(AircraftRadar,thetaFOR)
    DEF_READWRITE(AircraftRadar,track)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,AircraftRadar)

    expose_entity_subclass<MWS>(m,"MWS")
    DEF_READWRITE(MWS,isEsmIsh)
    DEF_READWRITE(MWS,Lref)
    DEF_READWRITE(MWS,thetaFOR)
    DEF_READWRITE(MWS,track)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,MWS)

    expose_entity_subclass<IRST>(m,"IRST")
    DEF_READWRITE(IRST,Lref)
    DEF_READWRITE(IRST,thetaFOR)
    DEF_READWRITE(IRST,track)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,IRST)

    expose_entity_subclass<MissileSensor>(m,"MissileSensor")
    DEF_READWRITE(MissileSensor,Lref)
    DEF_READWRITE(MissileSensor,thetaFOR)
    DEF_READWRITE(MissileSensor,thetaFOV)
    DEF_READWRITE(MissileSensor,estTPos)
    DEF_READWRITE(MissileSensor,track)
    DEF_READWRITE(MissileSensor,target)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,MissileSensor)
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
