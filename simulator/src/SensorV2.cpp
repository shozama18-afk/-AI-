// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "SensorV2.h"
#include "SimulationManager.h"
#include "Utility.h"
#include "Units.h"
#include "Fighter.h"
#include "Missile.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/nil_generator.hpp>

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

using namespace util;

void AircraftRadarV2::initialize(){
    BaseType::initialize();
    pfa=getValueFromJsonKRD(modelConfig,"pfa",randomGen,0.0);
    pl=getValueFromJsonKRD(modelConfig,"pl",randomGen,0.0);
    nTry=getValueFromJsonKRD(modelConfig,"nTry",randomGen,1);
    nNeedDetect=getValueFromJsonKRD(modelConfig,"nNeedDetect",randomGen,1);
    nMemory=getValueFromJsonKRD(modelConfig,"nMemory",randomGen,1);
    nNeedMemory=getValueFromJsonKRD(modelConfig,"nNeedMemory",randomGen,1);
    angleErrorMean=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorMean",randomGen,0.0));
    angleErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorSigma",randomGen,0.0));
    rangeErrorMean=getValueFromJsonKRD(modelConfig,"rangeErrorMean",randomGen,0.0);
    rangeErrorSigma=getValueFromJsonKRD(modelConfig,"rangeErrorSigma",randomGen,0.0);
    rVelErrorMean=getValueFromJsonKRD(modelConfig,"rVelErrorMean",randomGen,0.0);
    rVelErrorSigma=getValueFromJsonKRD(modelConfig,"rVelErrorSigma",randomGen,0.0);
    cVelErrorMean=getValueFromJsonKRD(modelConfig,"cVelErrorMean",randomGen,0.0);
    cVelErrorSigma=getValueFromJsonKRD(modelConfig,"cVelErrorSigma",randomGen,0.0);
    lookRCSScale=getValueFromJsonKRD(modelConfig,"lookRCSScale",randomGen,false);
    if(Lref<0 || pfa<=0 || pl<=0 || pfa>1 || pl>1){
        snCoeff=0.0;
    }else{
        snCoeff=(log(pfa)/log(pl)-1.0)*pow(Lref,4);
    }
}

void AircraftRadarV2::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,pfa
            ,pl
            ,angleErrorMean
            ,angleErrorSigma
            ,rangeErrorMean
            ,rangeErrorSigma
            ,rVelErrorMean
            ,rVelErrorSigma
            ,cVelErrorMean
            ,cVelErrorSigma
            ,nTry
            ,nNeedDetect
            ,nMemory
            ,nNeedMemory
            ,lookRCSScale
            ,snCoeff
        );
    }
    ASRC_SERIALIZE_NVP(archive
        ,candidates
        ,memoryCount
        ,candidatesMemoryCount
    );
}

void AircraftRadarV2::perceive(bool inReset){
    Sensor3D::perceive(inReset);
    if(isActive()){
        double dt=interval[SimPhase::PERCEIVE]*manager->getBaseTimeStep();
        // 航跡は親(機体座標系)上で保持しているが、Track3D::velはFREE_VECTOR(慣性速度をcrsの軸で表したもの)である。
        // 親に固定されている場合、crs上での見かけの速度は自機の並進・回転を差し引いたものになるので、
        // それを用いて外挿し、速度ベクトルもcrsの回転分だけ回す。
        std::function<void(Track3D&)> extrapolateTrack;
        if(isBoundToParent){
            auto trackCRS=getParentCRS();
            auto rootCRS=manager->getRootCRS();
            Eigen::Vector3d myPosInTrackCRS=pos(trackCRS);
            Eigen::Vector3d myVelFree=vel.transformTo(rootCRS).asTypeOf(CoordinateType::FREE_VECTOR)(trackCRS);
            Eigen::Vector3d myOmegaFree=omega.transformTo(rootCRS).asTypeOf(CoordinateType::FREE_VECTOR)(trackCRS);
            extrapolateTrack=[=](Track3D& t){
                Eigen::Vector3d p=t.pos();
                Eigen::Vector3d v=t.vel();
                Eigen::Vector3d vApparent=v-myVelFree-myOmegaFree.cross(p-myPosInTrackCRS);
                t.setPos(Eigen::Vector3d(p+vApparent*dt));
                t.setVel(Eigen::Vector3d(v-myOmegaFree.cross(v)*dt));
                t.setTime(t.time+dt);
            };
        }else{
            extrapolateTrack=[=](Track3D& t){
                t.updateByExtrapolation(dt);
            };
        }
        for(int i=0;i<track.size();++i){
            memoryCount[i].block(0,0,nMemory-1,1)=memoryCount[i].block(1,0,nMemory-1,1);
            memoryCount[i](nMemory-1)=0;
            extrapolateTrack(track[i]);
        }
        for(int i=0;i<candidates.size();++i){
            candidatesMemoryCount[i].block(0,0,nMemory-1,1)=candidatesMemoryCount[i].block(1,0,nMemory-1,1);
            candidatesMemoryCount[i](nMemory-1)=0;
            extrapolateTrack(candidates[i]);
        }
        auto crs=getParentCRS();
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Fighter>(asset);
        })){
            auto f=getShared<Fighter>(e);
            if(f->isAlive()){
                Eigen::Vector3d fpos=f->pos(crs);
                Eigen::Vector3d rpos=absPtoB(fpos);
                double L=rpos.norm();
                int count=0;
                if(thetaFOR<0 || rpos(0)>L*cos(thetaFOR)){
                    if(snCoeff>0){
                        double pd;
                        if(lookRCSScale){
                            pd=pow(pfa,1.0/(snCoeff*f->rcsScale/pow(L,4)+1.0));
                        }else{
                            pd=pow(pfa,1.0/(snCoeff/pow(L,4)+1.0));
                        }
                        std::uniform_real_distribution<double> dist(0.0,1.0);
                        for(int i=0;i<nTry;++i){
                            double p=dist(randomGen);
                            if(p<=pd){
                                count++;
                            }
                        }
                    }else{
                        if(Lref<0 || L<=Lref){
                            count=nNeedDetect;
                        }
                    }
                }
                if(count>=nNeedDetect){
                    // Track3D::velはFREE_VECTORなので、目標の速度もFREE_VECTORとしてcrsに変換する。
                    // f->vel(crs)はVELOCITYとして変換され、crs(機体座標系)の運動が差し引かれてしまう。
                    Eigen::Vector3d fvel=f->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs);
                    double dAngle= angleErrorSigma<=0 ? angleErrorMean : std::normal_distribution<double>(angleErrorMean,angleErrorSigma)(randomGen);
                    double dRange= rangeErrorSigma<=0 ? rangeErrorMean : std::normal_distribution<double>(rangeErrorMean,rangeErrorSigma)(randomGen);
                    double dRVel= rVelErrorSigma<=0 ? rVelErrorMean : std::normal_distribution<double>(rVelErrorMean,rVelErrorSigma)(randomGen);
                    double dCVel= cVelErrorSigma<=0 ? cVelErrorMean : std::normal_distribution<double>(cVelErrorMean,cVelErrorSigma)(randomGen);
                    double phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    double phiVel=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    Eigen::Vector3d myPos=pos(crs);
                    Eigen::Vector3d ex=(fpos-myPos)/L;
                    Eigen::Vector3d ey=getOrthogonalVector(ex);
                    Eigen::Vector3d ez=ex.cross(ey);
                    Eigen::Vector3d ax=cos(phi)*ey+sin(phi)*ez;
                    Quaternion q=Quaternion::fromAngle(ax,dAngle);
                    Eigen::Vector3d dPos=q.transformVector((L+dRange)*ex);
                    Eigen::Vector3d axVel=cos(phiVel)*ey+sin(phiVel)*ez;
                    Eigen::Vector3d dVel=dRVel*ex+dCVel*axVel;
                    bool found=false;
                    for(auto&& t:track){
                        if(t.isSame(f)){
                            t.update(Track3D(f,crs,manager->getTime(),myPos+dPos,fvel+dVel));
                            found=true;
                        }
                    }
                    if(!found){
                        found=false;
                        for(auto&& c:candidates){
                            if(c.isSame(f)){
                                c.update(Track3D(f,crs,manager->getTime(),myPos+dPos,fvel+dVel));
                                found=true;
                            }
                        }
                        if(!found){
                            candidates.push_back(Track3D(f,crs,manager->getTime(),myPos+dPos,fvel+dVel));
                            candidatesMemoryCount.push_back(Eigen::VectorXi::Zero(nMemory));
                        }
                    }
                }
                for(int i=0;i<track.size();++i){
                    if(track[i].isSame(f)){
                        memoryCount[i](nMemory-1)=count;
                    }
                }
                for(int i=0;i<candidates.size();++i){
                    if(candidates[i].isSame(f)){
                        candidatesMemoryCount[i](nMemory-1)=count;
                    }
                }
            }
        }
        for(int i=0;i<candidates.size();++i){
            if(candidatesMemoryCount[i].sum()>=nNeedMemory){
                bool found=false;
                for(auto&& t:track){
                    if(t.isSame(candidates[i])){
                        found=true;
                    }
                }
                if(!found){
                    track.push_back(candidates[i]);
                    memoryCount.push_back(candidatesMemoryCount[i]);
                }
            }
        }
        auto tit=track.begin();
        for(auto cit=memoryCount.begin();cit!=memoryCount.end();){
            if((*cit).sum() < nNeedMemory){
                cit=memoryCount.erase(cit);
                tit=track.erase(tit);
            }else{
                ++cit;
                ++tit;
            }
        }
    }
    observables["track"]=track;
}

void IRSTV2::initialize(){
    BaseType::initialize();
    angleErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorSigma",randomGen,0.0));
    omegaErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"omegaErrorSigma",randomGen,0.0));
    omegaDirectionErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"omegaDirectionErrorSigma",randomGen,0.0));
}

void IRSTV2::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,angleErrorSigma
            ,omegaErrorSigma
            ,omegaDirectionErrorSigma
        );
    }
}

void IRSTV2::perceive(bool inReset){
    Sensor2D::perceive(inReset);
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

                    // 方向への誤差付与
                    double dAngle = angleErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,angleErrorSigma)(randomGen);
                    Eigen::Vector3d ex=dir;
                    Eigen::Vector3d ey=getOrthogonalVector(ex);
                    Eigen::Vector3d ez=ex.cross(ey);
                    double phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    Eigen::Vector3d ax=cos(phi)*ey+sin(phi)*ez;
                    Quaternion q=Quaternion::fromAngle(ax,dAngle);
                    dir=q.transformVector(ex).normalized();

                    // 方向変化率への誤差付与
                    double dOmega = omegaErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,omegaErrorSigma)(randomGen);
                    double dOmegaDir = omegaDirectionErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,omegaDirectionErrorSigma)(randomGen);
                    ex=omega.normalized();
                    ey=getOrthogonalVector(ex);
                    ez=ex.cross(ey);
                    phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    ax=cos(phi)*ey+sin(phi)*ez;
                    q=Quaternion::fromAngle(ax,dOmegaDir);
                    omega=q.transformVector((omega.norm()+dOmega)*ex);

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

void MWSV2::initialize(){
    BaseType::initialize();
    angleErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorSigma",randomGen,0.0));
    omegaErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"omegaErrorSigma",randomGen,0.0));
    omegaDirectionErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"omegaDirectionErrorSigma",randomGen,0.0));
}

void MWSV2::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,angleErrorSigma
            ,omegaErrorSigma
            ,omegaDirectionErrorSigma
        );
    }
}
void MWSV2::perceive(bool inReset){
    Sensor2D::perceive(inReset);
    track.clear();
    if(isActive()){
        auto crs=getParentCRS();
        Eigen::Vector3d myPos=pos(crs);
        // Track2D::omegaはFREE_VECTORなので、視線角速度は慣性系での相対速度から求め、crsの軸で表す。
        Eigen::Vector3d myVel=vel.transformTo(manager->getRootCRS()).asTypeOf(CoordinateType::FREE_VECTOR)(crs);
        for(auto&& e:manager->getAssets([&](const std::shared_ptr<const Asset>& asset)->bool {
            return asset->getTeam()!=getTeam() && isinstance<Missile>(asset);
        })){
            auto m=getShared<Missile>(e);
            if(m->hasLaunched && m->isAlive()){
                Eigen::Vector3d mpos=m->pos(crs);
                Eigen::Vector3d rpos=absPtoB(mpos);
                double L=rpos.norm();
                bool detect=false;
                if(isEsmIsh){
                    if(m->sensor.lock()->isActive()){
                        std::pair<bool,Track3D> r=m->sensor.lock()->isTracking(Track3D(parent,crs));
                        if(r.first && (thetaFOR<0 || rpos(0)>=L*cos(thetaFOR)) && (Lref<0 || L<=Lref)){
                            detect=true;
                        }
                    }
                }else{
                    if((thetaFOR<0 || rpos(0)>=L*cos(thetaFOR)) && (Lref<0 || L<=Lref)){
                        detect=true;
                    }
                }

                if(detect){
                    // まずは真値を計算
                    Eigen::Vector3d dir=(mpos-myPos)/L;
                    Eigen::Vector3d omega=dir.cross(m->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs)-myVel)/L;

                    // 方向への誤差付与
                    double dAngle = angleErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,angleErrorSigma)(randomGen);
                    Eigen::Vector3d ex=dir;
                    Eigen::Vector3d ey=getOrthogonalVector(ex);
                    Eigen::Vector3d ez=ex.cross(ey);
                    double phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    Eigen::Vector3d ax=cos(phi)*ey+sin(phi)*ez;
                    Quaternion q=Quaternion::fromAngle(ax,dAngle);
                    dir=q.transformVector(ex).normalized();

                    // 方向変化率への誤差付与
                    double dOmega = omegaErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,omegaErrorSigma)(randomGen);
                    double dOmegaDir = omegaDirectionErrorSigma<=0 ? 0 : std::normal_distribution<double>(0,omegaDirectionErrorSigma)(randomGen);
                    ex=omega.normalized();
                    ey=getOrthogonalVector(ex);
                    ez=ex.cross(ey);
                    phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    ax=cos(phi)*ey+sin(phi)*ez;
                    q=Quaternion::fromAngle(ax,dOmegaDir);
                    omega=q.transformVector((omega.norm()+dOmega)*ex);

                    track.push_back(Track2D(m,crs,manager->getTime(),dir,myPos,omega));
                }
            }
        }
    }
    observables["track"]=track;
}

void MissileSensorV2::initialize(){
    BaseType::initialize();
    angleErrorMean=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorMean",randomGen,0.0));
    angleErrorSigma=deg2rad(getValueFromJsonKRD(modelConfig,"angleErrorSigma",randomGen,0.0));
    rangeErrorMean=getValueFromJsonKRD(modelConfig,"rangeErrorMean",randomGen,0.0);
    rangeErrorSigma=getValueFromJsonKRD(modelConfig,"rangeErrorSigma",randomGen,0.0);
    rVelErrorMean=getValueFromJsonKRD(modelConfig,"rVelErrorMean",randomGen,0.0);
    rVelErrorSigma=getValueFromJsonKRD(modelConfig,"rVelErrorSigma",randomGen,0.0);
    cVelErrorMean=getValueFromJsonKRD(modelConfig,"cVelErrorMean",randomGen,0.0);
    cVelErrorSigma=getValueFromJsonKRD(modelConfig,"cVelErrorSigma",randomGen,0.0);
}

void MissileSensorV2::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,angleErrorMean
            ,angleErrorSigma
            ,rangeErrorMean
            ,rangeErrorSigma
            ,rVelErrorMean
            ,rVelErrorSigma
            ,cVelErrorMean
            ,cVelErrorSigma
        )
    }
}

void MissileSensorV2::perceive(bool inReset){
    Sensor3D::perceive(inReset);
    bool isSelfTrackingTarget=isTracking(target).first;
    track.clear();
    if(isActive()){
        auto crs=getParentCRS();
        target=target.extrapolateTo(manager->getTime());
        estTPos=target.pos.transformTo(crs);
        Eigen::Vector3d myPos=pos(crs);
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
                    Eigen::Vector3d fvel=f->vel.asTypeOf(CoordinateType::FREE_VECTOR)(crs);
                    double dAngle= angleErrorSigma<=0 ? angleErrorMean : std::normal_distribution<double>(angleErrorMean,angleErrorSigma)(randomGen);
                    double dRange= rangeErrorSigma<=0 ? rangeErrorMean : std::normal_distribution<double>(rangeErrorMean,rangeErrorSigma)(randomGen);
                    double dRVel= rVelErrorSigma<=0 ? rVelErrorMean : std::normal_distribution<double>(rVelErrorMean,rVelErrorSigma)(randomGen);
                    double dCVel= cVelErrorSigma<=0 ? cVelErrorMean : std::normal_distribution<double>(cVelErrorMean,cVelErrorSigma)(randomGen);
                    double phi=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    double phiVel=std::uniform_real_distribution<double>(-M_PI,M_PI)(randomGen);
                    Eigen::Vector3d ex=(fpos-myPos)/L;
                    Eigen::Vector3d ey=getOrthogonalVector(ex);
                    Eigen::Vector3d ez=ex.cross(ey);
                    Eigen::Vector3d ax=cos(phi)*ey+sin(phi)*ez;
                    Quaternion q=Quaternion::fromAngle(ax,dAngle);
                    Eigen::Vector3d dPos=q.transformVector((L+dRange)*ex);
                    Eigen::Vector3d axVel=cos(phiVel)*ey+sin(phiVel)*ez;
                    Eigen::Vector3d dVel=dRVel*ex+dCVel*axVel;
                    track.push_back(Track3D(f,crs,manager->getTime(),myPos+dPos,fvel+dVel));
                }
            }
        }
    }
    observables["track"]=track;
}

void exportSensorV2(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;
    expose_entity_subclass<AircraftRadarV2>(m,"AircraftRadarV2")
    DEF_READWRITE(AircraftRadarV2,pfa)
    DEF_READWRITE(AircraftRadarV2,pl)
    DEF_READWRITE(AircraftRadarV2,nTry)
    DEF_READWRITE(AircraftRadarV2,nNeedDetect)
    DEF_READWRITE(AircraftRadarV2,nMemory)
    DEF_READWRITE(AircraftRadarV2,nNeedMemory)
    DEF_READWRITE(AircraftRadarV2,angleErrorMean)
    DEF_READWRITE(AircraftRadarV2,angleErrorSigma)
    DEF_READWRITE(AircraftRadarV2,rangeErrorMean)
    DEF_READWRITE(AircraftRadarV2,rangeErrorSigma)
    DEF_READWRITE(AircraftRadarV2,rVelErrorMean)
    DEF_READWRITE(AircraftRadarV2,rVelErrorSigma)
    DEF_READWRITE(AircraftRadarV2,cVelErrorMean)
    DEF_READWRITE(AircraftRadarV2,cVelErrorSigma)
    DEF_READWRITE(AircraftRadarV2,candidates)
    DEF_READWRITE(AircraftRadarV2,memoryCount)
    DEF_READWRITE(AircraftRadarV2,candidatesMemoryCount)
    DEF_READWRITE(AircraftRadarV2,snCoeff)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,AircraftRadarV2)

    expose_entity_subclass<IRSTV2>(m,"IRSTV2")
    DEF_READWRITE(IRSTV2,angleErrorSigma)
    DEF_READWRITE(IRSTV2,omegaErrorSigma)
    DEF_READWRITE(IRSTV2,omegaDirectionErrorSigma)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,IRSTV2)

    expose_entity_subclass<MWSV2>(m,"MWSV2")
    DEF_READWRITE(MWSV2,angleErrorSigma)
    DEF_READWRITE(MWSV2,omegaErrorSigma)
    DEF_READWRITE(MWSV2,omegaDirectionErrorSigma)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,MWSV2)

    expose_entity_subclass<MissileSensorV2>(m,"MissileSensorV2")
    DEF_READWRITE(MissileSensorV2,angleErrorMean)
    DEF_READWRITE(MissileSensorV2,angleErrorSigma)
    DEF_READWRITE(MissileSensorV2,rangeErrorMean)
    DEF_READWRITE(MissileSensorV2,rangeErrorSigma)
    DEF_READWRITE(MissileSensorV2,rVelErrorMean)
    DEF_READWRITE(MissileSensorV2,rVelErrorSigma)
    DEF_READWRITE(MissileSensorV2,cVelErrorMean)
    DEF_READWRITE(MissileSensorV2,cVelErrorSigma)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,MissileSensorV2)
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
