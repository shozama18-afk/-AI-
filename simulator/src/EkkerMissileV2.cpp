// Copyright (c) 2023-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "EkkerMissileV2.h"
#include <boost/math/tools/roots.hpp>
#include "MathUtility.h"
#include "Units.h"
#include "SimulationManager.h"
#include "EkkerMDTV2.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
using namespace util;

void EkkerMissileV2::initialize(){
    BaseType::initialize();
    taperRatioW=getValueFromJsonKRD(modelConfig,"taperRatioW",randomGen,0.0);
    taperRatioT=getValueFromJsonKRD(modelConfig,"taperRatioT",randomGen,0.0);
    isTailControlled=getValueFromJsonKRD(modelConfig,"isTailControlled",randomGen,true);
}
void EkkerMissileV2::serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full){
    BaseType::serializeInternalState(archive,full);

    if(full){
        ASRC_SERIALIZE_NVP(archive
            ,taperRatioW
            ,taperRatioT
            ,isTailControlled
        )
    }
}

std::pair<double,Eigen::Vector3d> EkkerMissileV2::calcAccel(const Eigen::Vector3d& accelCmd, double tAftLaunch,double dt){
    double V=motion.vel.norm();
    double alt=motion.getHeight();
    bool pwroff=tAftLaunch>=tBurn;
    double thrust_;
    if(pwroff){
        thrust_ = 0.0;
        mass = massF;
        lcg = lcgf;
    }else{
        thrust_ = thrust;
        mass = massI+(massF-massI)*tAftLaunch/tBurn;
        lcg = lcgi+(lcgf-lcgi)*tAftLaunch/tBurn;
    }
    std::map<std::string,double> atm=atmosphere(alt);
    double M=V/atm["a"];
    double qd=0.5*atm["rho"]*V*V;
    Eigen::Vector3d ex=motion.vel()/V;
    Eigen::Vector3d gravityAccelVector=motion.dirHtoP(Eigen::Vector3d(0,0,1),"NED")*gravity; //TODO 慣性力モデルに置換
    Eigen::Vector3d ey=(accelCmd-gravityAccelVector).cross(ex)*mass;
    double desiredSideForce=ey.norm();
    Eigen::Vector3d sideAx;
    double aoa,CL,CD;
    if(desiredSideForce>0){
        ey/=desiredSideForce;
        sideAx=ex.cross(ey);
        std::pair<double,double> tmp=EkkerMDTV2::cldcmd(d,l,ln,lcg,bw,Sw,taperRatioW,lw,bt,St,taperRatioT,M,isTailControlled);
        double cld=tmp.first;
        double cmd=tmp.second;
        auto func=[&](double aoa_){
            std::pair<double,double> tmp=EkkerMDTV2::clacma(d,l,ln,lcg,bw,Sw,taperRatioW,lw,bt,St,taperRatioT,M,aoa_);
            double cla=tmp.first;
            double cma=tmp.second;
            double delta=std::clamp<double>(-cma/cmd*aoa_,-maxD,maxD);
            return (cla*aoa_+cld*delta)*(qd*Sref)+thrust_*sin(aoa_)-desiredSideForce;
        };
        if(func(0.0)>=0){
            aoa=0.0;
        }else if(func(maxA)<=0){
            aoa=maxA;
        }else{
            try{
                boost::math::tools::eps_tolerance<double> tol(16);
                boost::uintmax_t max_iter=10;
                auto result=boost::math::tools::toms748_solve(func,0.0,maxA,tol,max_iter);
                aoa=(result.first+result.second)/2;
            }catch(std::exception& e){
                std::cout<<"exception at EkkerMissileV2::calcMotion"<<std::endl;
                DEBUG_PRINT_EXPRESSION(func(0.0))
                DEBUG_PRINT_EXPRESSION(func(maxA))
                DEBUG_PRINT_EXPRESSION(alt)
                DEBUG_PRINT_EXPRESSION(M)
                DEBUG_PRINT_EXPRESSION(desiredSideForce)
                DEBUG_PRINT_EXPRESSION(thrust_)
                throw e;
            }
        }
        tmp=EkkerMDTV2::clacma(d,l,ln,lcg,bw,Sw,taperRatioW,lw,bt,St,taperRatioT,M,aoa);
        double cla=tmp.first;
        double cma=tmp.second;
        double delta=std::clamp<double>(-cma/cmd*aoa,-maxD,maxD);
        //CL=(cla-cld*cma/cmd)*aoa;
        CL=cla*aoa+cld*delta;
        CD=EkkerMDTV2::cdtrim(d,l,ln,bw,Sw,thicknessRatio,taperRatioW,lw,bt,St,thicknessRatio,taperRatioT,M,alt,aoa,delta,frictionFactor,pwroff,isTailControlled);
    }else{
        sideAx<<0,0,0;
        CL=0.0;
        CD=EkkerMDTV2::cdtrim(d,l,ln,bw,Sw,thicknessRatio,taperRatioW,lw,bt,St,thicknessRatio,taperRatioT,M,alt,0,0,frictionFactor,pwroff,isTailControlled);
        aoa=0.0;
    }
    Eigen::Vector3d sideAccel=sideAx*(CL*qd*Sref+thrust_*sin(aoa))/mass+(gravityAccelVector-ex*(gravityAccelVector.dot(ex)));
	//attitude and deflection are assumed to be immidiately adjustable as desired.
	double accelScalar=(-CD*Sref*qd+thrust_*cos(aoa))/mass+gravityAccelVector.dot(ex);
    Eigen::Vector3d omegaV=motion.vel().cross(sideAccel)/(V*V);
    return {accelScalar,omegaV};
}

void exportEkkerMissileV2(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper)
{
    using namespace pybind11::literals;
    auto cls=expose_entity_subclass<EkkerMissileV2>(m,"EkkerMissileV2");
    cls
    DEF_READWRITE(EkkerMissileV2,taperRatioW)
    DEF_READWRITE(EkkerMissileV2,taperRatioT)
    DEF_READWRITE(EkkerMissileV2,isTailControlled)
    ;
    FACTORY_ADD_CLASS(PhysicalAsset,EkkerMissileV2)
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
