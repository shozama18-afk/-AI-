// Copyright (c) 2025-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "R8ContestMassPointFighter.h"
#include <ASRCAISim1/Utility.h>
#include <ASRCAISim1/Units.h>
#include <ASRCAISim1/SimulationManager.h>
#include <ASRCAISim1/MassPointFighter.h>

ASRC_PLUGIN_NAMESPACE_BEGIN

using namespace asrc::core;
using namespace util;

void R8ContestMassPointFighter::initialize(){
    BaseType::initialize();
    if(modelConfig.at("dynamics").contains("table")){
        nl::json& j_table=modelConfig.at("dynamics").at("table");
        if(j_table.is_string()){
            // .npz file
            py::gil_scoped_acquire acquire;
            std::string moduleName=py::cast<std::string>(py::cast(this->shared_from_this()).attr("__module__"));
            auto np=py::module_::import("numpy");
            py::object loaded;
            std::filesystem::path filePath=getFactoryHelperChain().resolveFilePath(modelConfig.at("dynamics").at("table").get<std::string>());
            if(std::filesystem::exists(filePath)){
                loaded=np.attr("load")(filePath.generic_string());
            }else{
                throw std::runtime_error("The dynamics table file \""+filePath.generic_string()+"\"is not found.");
            }
            altTable=py::cast<Eigen::VectorXd>(loaded["altitude"]);
            vMinTable.setData(
                py::cast<Eigen::Tensor<double,1,Eigen::RowMajor>>(loaded["vMin"]),
                altTable
            );
            vMinTable.check(true);
            vMaxTable.setData(
                py::cast<Eigen::Tensor<double,1,Eigen::RowMajor>>(loaded["vMax"]),
                altTable
            );
            vMaxTable.check(true);
            std::vector<std::string> keys{
                "accelMin",
                "accelMax",
                "rollMin",
                "rollMax",
                "pitchMin",
                "pitchMax",
                "yawMin",
                "yawMax"
            };
            for(auto&& key : keys){
                Eigen::Tensor<double,2,Eigen::RowMajor> tableValue2D=py::cast<Eigen::Tensor<double,2,Eigen::RowMajor>>(loaded[key.c_str()]);
                Eigen::VectorXd vRatioTable=Eigen::VectorXd::LinSpaced(tableValue2D.dimension(1),0.0,1.0);
                twoDimTables.emplace(std::piecewise_construct,
                    std::forward_as_tuple(key),
                    std::forward_as_tuple(tableValue2D,altTable,vRatioTable)
                );
                twoDimTables[key].check();
            }
        }else if(j_table.is_object()){
            loadTableFromJsonObject(j_table);
        }else{
            throw std::runtime_error("The value of 'table' should be string or object.");
        }
    }else{
        loadTableFromJsonObject(modelConfig.at("dynamics"));
    }

    {
        std::vector<std::string> keys{
            "vMin",
            "vMax",
            "accelMin",
            "accelMax",
            "rollMin",
            "rollMax",
            "pitchMin",
            "pitchMax",
            "yawMin",
            "yawMax"
        };
        if(modelConfig.at("dynamics").contains("scale")){
            const nl::json& j=modelConfig.at("dynamics").at("scale");
            for(auto&& key : keys){
                if(j.contains(key)){
                    scaleFactor[key]=getValueFromJsonKR(j,key,randomGen);
                }else{
                    scaleFactor[key]=1.0;
                }
            }
        }else{
            for(auto&& key : keys){
                scaleFactor[key]=1.0;
            }
        }
    }

    observables["spec"]["dynamics"]={
        {"scale",scaleFactor}
    };
}

void R8ContestMassPointFighter::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,altTable
            ,vMinTable
            ,vMaxTable
            ,twoDimTables
            ,scaleFactor
        )
    }
}

double R8ContestMassPointFighter::getThrust(){
    auto [accelMin,accelMax] = getAccelRange(motion);
    nl::json ctrl=controllers["FlightController"].lock()->commands["motion"];
    double pCmd=ctrl.at("pCmd");

    // 重力の影響を反映
    double V=motion.vel.norm();
    Eigen::Vector3d vn=motion.vel()/V;
    Eigen::Vector3d gravityAxis=motion.dirHtoP(Eigen::Vector3d(0,0,1),Eigen::Vector3d(0,0,0),"NED",false);
    double gOfs=vn.dot(gravityAxis)*gravity;

    return m*(accelMin+pCmd*(accelMax-accelMin)+gOfs);
}

void R8ContestMassPointFighter::calcMotion(double dt){
    motion_prev=motion;
    double V=motion.vel.norm();
    nl::json ctrl=controllers["FlightController"].lock()->commands["motion"];
    Eigen::Vector3d omegaB=ctrl.at("omegaB");
    auto extra=getExtraDynamicsState();
    auto [vMin,vMax]=getVelocityRange(motion,extra);
    auto [rollMin,rollMax]=getRollRateRange(motion,extra);
    auto [pitchMin,pitchMax]=getPitchRateRange(motion,extra);
    auto [yawMin,yawMax]=getYawRateRange(motion,extra);
    omegaB(0)=std::clamp(omegaB(0),rollMin,rollMax);
    omegaB(1)=std::clamp(omegaB(1),pitchMin,pitchMax);
    omegaB(2)=std::clamp(omegaB(2),yawMin,yawMax);
    double accel=getThrust()/m;
    motion.setOmega(relBtoP(omegaB));
    double theta=motion.omega.norm()*dt;
    if(theta>1e-6){
        Eigen::Vector3d ax=motion.omega.normalized()();
        Quaternion dq=Quaternion::fromAngle(ax,theta);
        motion.setQ(dq*motion.q);
    }else{
        Eigen::VectorXd dq=motion.q.dqdwi()*motion.omega()*dt;
        motion.setQ((motion.q+Quaternion(dq)).normalized());
    }
    double Vaft=std::max(vMin,std::min(vMax,V+accel*dt));

    // 高度に天井を設ける
    Eigen::Vector3d newVel=relBtoP(Eigen::Vector3d(Vaft,0,0));
    Eigen::Vector3d newPos=motion.pos()+newVel*dt;
    double newAlt=motion.getCRS()->getHeight(newPos,motion.time+dt);
    double altMax=altTable(altTable.size()-1);
    if(newAlt>altMax){
        // 天井に当たる場合は速度を維持しつつ、
        // このフレームの移動先の高度が上限値になるように移動方向を強制的に曲げる
        // ここでは角速度制限を無視する

        // NED座標系での補正後速度ベクトルの向きを求める
        Eigen::Vector3d velNED=relPtoH(newVel,motion.pos(),"NED",false);
        Eigen::Vector3d dirNE(velNED(0),velNED(1),0);
        if(dirNE.norm()>0){
            dirNE.normalize();
        }else{
            // 完全な垂直上昇の場合は機種上げ方向に曲げる
            dirNE=dirBtoH(Eigen::Vector3d(0,0,-1),motion.pos(),"NED",false);
        }

        // 補正後の姿勢と角速度ベクトルを求める
        Eigen::Vector3d oldEx=newVel.normalized();
        Eigen::Vector3d newEx=dirHtoP(dirNE,Eigen::Vector3d(0,0,0),"NED",false);
        Eigen::Vector3d ax=oldEx.cross(newEx);
        double cs=std::clamp<double>(oldEx.dot(newEx),-1,1);
        double sn=std::clamp<double>(ax.norm(),0,1);
        double theta=atan2(sn,cs);
        if(theta>1e-6){
            Quaternion dq=Quaternion::fromAngle(ax.normalized(),theta);
            motion.setQ(dq*motion.q);

            // 姿勢の差分に相当する角速度を逆算
            dq=motion.q*motion_prev.q.conjugate();
            cs=dq.w();
            sn=dq.vec().norm();
            if(sn>0){
                ax=dq.vec()/sn;
                theta=atan2(sn,cs)*2;
                motion.setOmega(ax*theta/dt);
            }else{
                motion.setOmega(Eigen::Vector3d(0,0,0));
            }
        }else{
            // ほぼ補正なしの場合は、姿勢と角速度の補正は無視する
        }

        // P座標系での補正後速度ベクトルを求める
        double velD=-(altMax-motion.getHeight())/dt;
        double velNE=sqrt(Vaft*Vaft-velD*velD);
        Eigen::Vector3d newVelNED=velNE*dirNE+Eigen::Vector3d(0,0,velD);
        newVel=relHtoP(newVelNED,Eigen::Vector3d(0,0,0),"NED",false);

        // P座標系での補正後位置ベクトルを求める
        Eigen::Vector3d newPosNED=absPtoH(motion.pos(),motion.pos(),"NED",true)+newVelNED*dt;
        newPosNED(2)=-altMax;
        newPos=absHtoP(newPosNED,motion.pos(),"NED",true);
    }

    motion.setVel(newVel);
    motion.setPos(newPos);
}

nl::json R8ContestMassPointFighter::getExtraDynamicsState(){
    return nl::json::object();
}

std::pair<double,double> R8ContestMassPointFighter::getVelocityRange(const MotionState& m,const nl::json& extraArgs){
    double alt=m.getHeight();
    return {
        vMinTable(alt,true)*scaleFactor["vMin"],
        vMaxTable(alt,true)*scaleFactor["vMax"]
    };
}
double R8ContestMassPointFighter::getTwoDimTableValue(const std::string& key,const MotionState& m,const nl::json& extraArgs){
    double alt=m.getHeight();
    double V=m.vel.norm();
    auto [vMin,vMax]=getVelocityRange(m,extraArgs);
    double vRatio=(V-vMin)/(vMax-vMin);
    return twoDimTables[key](Eigen::VectorXd{{alt,vRatio}},true)*scaleFactor[key];
}

std::pair<double,double> R8ContestMassPointFighter::getAccelRange(const MotionState& m,const nl::json& extraArgs){
    return {getTwoDimTableValue("accelMin",m,extraArgs),getTwoDimTableValue("accelMax",m,extraArgs)};
}
std::pair<double,double> R8ContestMassPointFighter::getRollRateRange(const MotionState& m,const nl::json& extraArgs){
    return {getTwoDimTableValue("rollMin",m,extraArgs),getTwoDimTableValue("rollMax",m,extraArgs)};
}
std::pair<double,double> R8ContestMassPointFighter::getPitchRateRange(const MotionState& m,const nl::json& extraArgs){
    return {getTwoDimTableValue("pitchMin",m,extraArgs),getTwoDimTableValue("pitchMax",m,extraArgs)};
}
std::pair<double,double> R8ContestMassPointFighter::getYawRateRange(const MotionState& m,const nl::json& extraArgs){
    return {getTwoDimTableValue("yawMin",m,extraArgs),getTwoDimTableValue("yawMax",m,extraArgs)};
}

void R8ContestMassPointFighter::loadTableFromJsonObject(const nl::json& j_table){
    //
    if(j_table.contains("altitude")){
        const nl::json& j=j_table.at("altitude");
        if(j.is_array()){
            j.get_to(altTable);
            assert(altTable.size()>1);
        }else{
            throw std::runtime_error("The value of 'altitude' should be 1-dim array.");
        }
    }else{
        altTable=Eigen::Vector2d(0.0,20000.0);
    }

    Eigen::Tensor<double,1,Eigen::RowMajor> tableValue1D;
    if(j_table.contains("vMin")){
        const nl::json& j=j_table.at("vMin");
        if(j.is_array()){
            j.get_to(tableValue1D);
            if(tableValue1D.size()==1){
                tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(tableValue1D(0));
            }
        }else if(j.is_number()){
             tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(j.get<double>());
        }else{
            throw std::runtime_error("The value of 'vMin' should be 1-dim array or number.");
        }
    }else{
        tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(150.0);
    }
    vMinTable.setData(tableValue1D,altTable);
    vMinTable.check(true);

    if(j_table.contains("vMax")){
        const nl::json& j=j_table.at("vMax");
        if(j.is_array()){
            j.get_to(tableValue1D);
            if(tableValue1D.size()==1){
                tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(tableValue1D(0));
            }
        }else if(j.is_number()){
             tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(j.get<double>());
        }else{
            throw std::runtime_error("The value of 'vMax' should be 1-dim array or number.");
        }
    }else{
        tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(altTable.size()).setConstant(450.0);
    }
    vMaxTable.setData(tableValue1D,altTable);
    vMaxTable.check(true);

    std::vector<std::string> keys{
        "accelMin",
        "accelMax",
        "rollMin",
        "rollMax",
        "pitchMin",
        "pitchMax",
        "yawMin",
        "yawMax"
    };
    std::map<std::string,double> defaultValues{
        {"accelMin",-gravity},
        {"accelMax",gravity},
        {"rollMin",-deg2rad(180.0)},
        {"rollMax",deg2rad(180.0)},
        {"pitchMin",-deg2rad(30.0)},
        {"pitchMax",deg2rad(30.0)},
        {"yawMin",-deg2rad(30.0)},
        {"yawMax",deg2rad(30.0)}
    };
    for(auto&& key : keys){
        Eigen::Tensor<double,2,Eigen::RowMajor> tableValue2D;
        if(j_table.contains(key)){
            const nl::json& j=j_table.at(key);
            if(j.is_array()){
                assert(j.size()>0);
                if(j[0].is_array()){
                    // 2-dim array
                    j.get_to(tableValue2D);
                }else if(j[0].is_number()){
                    // 1-dim array
                    j.get_to(tableValue1D);
                    if(tableValue1D.size()==1){
                        tableValue1D=Eigen::Tensor<double,1,Eigen::RowMajor>(2).setConstant(tableValue1D(0));
                    }
                    tableValue2D=tableValue1D.reshape(Eigen::DSizes<Eigen::Index,2>{tableValue1D.size(),1}).broadcast(Eigen::DSizes<Eigen::Index,2>{1,2});
                }else{
                    throw std::runtime_error("The value of '"+key+"' should be 2-dim array, 1-dim array or number.");
                }
                if(tableValue2D.size()==1){
                    tableValue2D=Eigen::Tensor<double,2,Eigen::RowMajor>(altTable.size(),2).setConstant(tableValue2D(0));
                }else if(tableValue2D.dimension(0)==1){
                    tableValue2D=tableValue2D.broadcast(Eigen::DSizes<Eigen::Index,2>{altTable.size(),1});
                }else if(tableValue2D.dimension(1)==1){
                    tableValue2D=tableValue2D.broadcast(Eigen::DSizes<Eigen::Index,2>{1,2});
                }
            }else if(j.is_number()){
                tableValue2D=Eigen::Tensor<double,2,Eigen::RowMajor>(altTable.size(),2).setConstant(j.get<double>());
            }else{
                throw std::runtime_error("The value of '"+key+"' should be 2-dim array, 1-dim array or number.");
            }
        }else{
            tableValue2D=Eigen::Tensor<double,2,Eigen::RowMajor>(altTable.size(),2).setConstant(defaultValues[key]);
        }
        Eigen::VectorXd vRatioTable=Eigen::VectorXd::LinSpaced(tableValue2D.dimension(1),0.0,1.0);
        twoDimTables.emplace(std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(tableValue2D,altTable,vRatioTable)
        );
        twoDimTables[key].check();
    }
}

std::shared_ptr<EntityAccessor> R8ContestMassPointFighter::getAccessorImpl(){
    return std::make_shared<R8ContestMassPointFighterAccessor>(getShared<R8ContestMassPointFighter>(this->shared_from_this()));
}

nl::json R8ContestMassPointFlightController::calcDirect(const nl::json &cmd){
    auto p=getShared<R8ContestMassPointFighter>(parent);
    auto extra=p->getExtraDynamicsState();
    auto [rollMin,rollMax]=p->getRollRateRange(p->motion,extra);
    auto [pitchMin,pitchMax]=p->getPitchRateRange(p->motion,extra);
    auto [yawMin,yawMax]=p->getYawRateRange(p->motion,extra);
    auto roll=std::clamp(cmd.at("roll").get<double>(),-1.,1.);
    if(roll>=0){
        if(rollMax>0){
            roll=std::max(rollMin,roll*abs(rollMax));
        }else{
            roll=rollMax;
        }
    }else{
        if(rollMin<0){
            roll=std::min(rollMax,roll*abs(rollMin));
        }else{
            roll=rollMin;
        }
    }
    auto pitch=std::clamp(cmd.at("pitch").get<double>(),-1.,1.);
    if(pitch>=0){
        if(pitchMax>0){
            pitch=std::max(pitchMin,pitch*abs(pitchMax));
        }else{
            pitch=pitchMax;
        }
    }else{
        if(pitchMin<0){
            pitch=std::min(pitchMax,pitch*abs(pitchMin));
        }else{
            pitch=pitchMin;
        }
    }
    auto yaw=std::clamp(cmd.at("yaw").get<double>(),-1.,1.);
    if(yaw>=0){
        if(yawMax>0){
            yaw=std::max(yawMin,yaw*abs(yawMax));
        }else{
            yaw=yawMax;
        }
    }else{
        if(yawMin<0){
            yaw=std::min(yawMax,yaw*abs(yawMin));
        }else{
            yaw=yawMin;
        }
    }
    double pCmd;
    if(cmd.contains("throttle")){//0〜1
        pCmd=std::clamp(cmd.at("throttle").get<double>(),0.,1.);
    }else if(cmd.contains("accel")){//-1〜+1
        double accel=std::clamp(cmd.at("accel").get<double>(),-1.,1.);
        auto [accelMin,accelMax]=p->getAccelRange(p->motion,extra);
        if(accel>=0){
            if(accelMax>0){
                accel=std::max(accelMin,accel*abs(accelMax));
            }else{
                accel=accelMax;
            }
        }else{
            if(accelMin<0){
                accel=std::min(accelMax,accel*abs(accelMin));
            }else{
                accel=accelMin;
            }
        }
        pCmd=(accel-accelMin)/(accelMax-accelMin);
    }else{
        throw std::runtime_error("Either 'throttle' or 'accel' must be in cmd keys.");
    }
    return {
        {"omegaB",Eigen::Vector3d(roll,pitch,yaw)},
        {"pCmd",pCmd}
    };
}
nl::json R8ContestMassPointFlightController::calcFromDirAndVel(const nl::json &cmd){
    auto p=getShared<R8ContestMassPointFighter>(parent);
    auto extra=p->getExtraDynamicsState();
    auto [accelMin,accelMax]=p->getAccelRange(p->motion,extra);
    double pCmd;
    if(cmd.contains("dstV")){
        double V=p->motion.vel.norm();
	    double dstV=cmd.at("dstV");
        double dt=p->interval[SimPhase::BEHAVE]*manager->getBaseTimeStep();
        double dstAccel=(dstV-V)/dt;
		pCmd=std::clamp((dstAccel-accelMin)/(accelMax-accelMin),0.0,1.0);
    }else if(cmd.contains("dstAccel")){
		double dstAccel=cmd.at("dstAccel");
		pCmd=std::clamp((dstAccel-accelMin)/(accelMax-accelMin),0.0,1.0);
    }else if(cmd.contains("dstThrust")){
		double dstT=cmd.at("dstThrust");
        double Tmin=accelMin*p->m;
        double Tmax=accelMax*p->m;
		pCmd=std::clamp((dstT-Tmin)/(Tmax-Tmin),0.0,1.0);
    }else if(cmd.contains("dstThrottle")){
        double dstT=cmd.at("dstThrottle");
		pCmd=std::clamp(dstT,0.0,1.0);
    }else{
		throw std::runtime_error("Only one of dstV, dstAccel, dstThrust or dstThrottle is acceptable.");
    }

    Eigen::Vector3d omegaB;
	if(cmd.contains("dstDir")){
		Eigen::Vector3d dstDir=cmd.at("dstDir");
        if(cmd.contains("dstAlt")){
            dstDir=altitudeKeeper(p->motion,dstDir,cmd.at("dstAlt").get<double>());
        }
        double V=p->motion.vel.norm();
        Eigen::Vector3d currentDir=p->motion.vel()/V;
		dstDir.normalize();
		Eigen::Vector3d rotationAxis=currentDir.cross(dstDir);
        auto ex=p->dirBtoP(Eigen::Vector3d(1,0,0));
        auto ey=p->dirBtoP(Eigen::Vector3d(0,1,0));
        auto ez=p->dirBtoP(Eigen::Vector3d(0,1,1));
        auto down=p->motion.dirHtoP(Eigen::Vector3d(0,0,1),Eigen::Vector3d(0,0,0),"NED",false);
		auto horizontalY=down.cross(ex);
		double snTheta=std::min(1.0,rotationAxis.norm());
        double csTheta=std::clamp(currentDir.dot(dstDir),-1.0,1.0);
        double eTheta=atan2(snTheta,csTheta);
        double eps=1e-4;
        double dt=p->interval[SimPhase::BEHAVE]*manager->getBaseTimeStep();
		if(snTheta<eps){
            //Almost no rotation or 180deg rotation
            rotationAxis=currentDir.cross(down.cross(currentDir)).normalized();
        }else{
            rotationAxis/=snTheta;
        }
        Quaternion dq=Quaternion::fromAngle(rotationAxis,eTheta);
        Quaternion q1=dq*p->motion.q;
        ex=q1.transformVector(Eigen::Vector3d(1,0,0));// == dstDir
        ey=q1.transformVector(Eigen::Vector3d(0,1,0));
        double sinRoll=ex.dot(ey.cross(rotationAxis));
        double cosRoll=ey.dot(rotationAxis);
        double roll=atan2(sinRoll,cosRoll);
        if(roll>M_PI_2){
            roll=M_PI-roll;
        }else if(roll<-M_PI_2){
            roll=M_PI+roll;
        }
        auto [rollMin,rollMax]=p->getRollRateRange(p->motion,extra);
        roll=std::clamp(roll,rollMin,rollMax);
        dq=Quaternion::fromAngle(ex,roll)*dq;
        if(dq.w()<0){
            dq=-dq;
        }
        double cosRot=dq.w();
        double sinRot=dq.vec().norm();
        Eigen::Vector3d ax=dq.vec().normalized();
        omegaB=p->relPtoB(ax*atan2(sinRot,cosRot)*2/dt);
        auto [pitchMin,pitchMax]=p->getPitchRateRange(p->motion,extra);
        auto [yawMin,yawMax]=p->getYawRateRange(p->motion,extra);
        omegaB*=std::min<double>({
            1.0,
            omegaB(0)==0 ? 1.0 : abs((omegaB(0)>0 ? rollMax : rollMin)/omegaB(0)),
            omegaB(1)==0 ? 1.0 : abs((omegaB(1)>0 ? pitchMax : pitchMin)/omegaB(1)),
            omegaB(2)==0 ? 1.0 : abs((omegaB(2)>0 ? yawMax : yawMin)/omegaB(2)),
        });
    }else if(cmd.contains("dstTurnRate")){
        omegaB=cmd.at("dstTurnRate");
		throw std::runtime_error("Only one of dstDir, or dstTurnRate is acceptable.");
    }

    return {
        {"omegaB",omegaB},
        {"pCmd",pCmd}
    };
}

R8ContestMassPointFighterAccessor::R8ContestMassPointFighterAccessor(const std::shared_ptr<R8ContestMassPointFighter>& f)
:FighterAccessor(f)
{
}
nl::json R8ContestMassPointFighterAccessor::getExtraDynamicsState(){
    return get<R8ContestMassPointFighter>(true)->getExtraDynamicsState();
}
std::pair<double,double> R8ContestMassPointFighterAccessor::getVelocityRange(const MotionState& m,const nl::json& extraArgs){
    return get<R8ContestMassPointFighter>(true)->getVelocityRange(m,extraArgs);
}
std::pair<double,double> R8ContestMassPointFighterAccessor::getAccelRange(const MotionState& m,const nl::json& extraArgs){
    return get<R8ContestMassPointFighter>(true)->getAccelRange(m,extraArgs);
}
std::pair<double,double> R8ContestMassPointFighterAccessor::getRollRateRange(const MotionState& m,const nl::json& extraArgs){
    return get<R8ContestMassPointFighter>(true)->getRollRateRange(m,extraArgs);
}
std::pair<double,double> R8ContestMassPointFighterAccessor::getPitchRateRange(const MotionState& m,const nl::json& extraArgs){
    return get<R8ContestMassPointFighter>(true)->getPitchRateRange(m,extraArgs);
}
std::pair<double,double> R8ContestMassPointFighterAccessor::getYawRateRange(const MotionState& m,const nl::json& extraArgs){
    return get<R8ContestMassPointFighter>(true)->getYawRateRange(m,extraArgs);
}

void exportR8ContestMassPointFighter(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;

    expose_entity_subclass<R8ContestMassPointFighter>(m,"R8ContestMassPointFighter")
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,R8ContestMassPointFighter)

    expose_entity_subclass<R8ContestMassPointFlightController>(m,"R8ContestMassPointFlightController")
    ;
    FACTORY_ADD_CLASS(Controller,R8ContestMassPointFlightController)

    expose_common_class<R8ContestMassPointFighterAccessor>(m,"R8ContestMassPointFighterAccessor")
    .def(py_init<const std::shared_ptr<R8ContestMassPointFighter>&>())
    DEF_FUNC(R8ContestMassPointFighterAccessor,getExtraDynamicsState)
    DEF_FUNC(R8ContestMassPointFighterAccessor,getVelocityRange)
    DEF_FUNC(R8ContestMassPointFighterAccessor,getAccelRange)
    DEF_FUNC(R8ContestMassPointFighterAccessor,getRollRateRange)
    DEF_FUNC(R8ContestMassPointFighterAccessor,getPitchRateRange)
    DEF_FUNC(R8ContestMassPointFighterAccessor,getYawRateRange)
    ;
}

ASRC_PLUGIN_NAMESPACE_END
