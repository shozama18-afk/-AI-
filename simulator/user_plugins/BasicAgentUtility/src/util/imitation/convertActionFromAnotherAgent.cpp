// Copyright (c) 2026-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "util/imitation/convertActionFromAnotherAgent.h"
#include <limits>

ASRC_PLUGIN_NAMESPACE_BEGIN

namespace util{
    namespace imitation{

using namespace asrc::core;
using namespace util;

double getExpertDstAZ(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    MotionState originalMyMotion(myObs.at("motion")); // 元のparent座標系での値も必要

    std::string decisionType=decision["Horizontal"][0];
    double value=decision["Horizontal"][1];
    nl::json motionCmd=command.at("motion");

    double dstAZ;
    if(decisionType=="Az_NED"){
        dstAZ=value;
    }else if(decisionType=="Az_BODY"){
        dstAZ=value+myMotion.getAZ();
    }else if(motionCmd.contains("dstDir")){
        Eigen::Vector3d dd=originalMyMotion.dirPtoH(motionCmd.at("dstDir"),originalMyMotion.pos(),"NED",false);
        dstAZ=atan2(dd(1),dd(0));
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDstAZ'.");
    }
    while(dstAZ > M_PI){
        dstAZ-=2*M_PI;
    }
    while(dstAZ <= -M_PI){
        dstAZ+=2*M_PI;
    }
    return dstAZ;
}

double getExpertDeltaAZ(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& refAZ
){
    double deltaAZ=getExpertDstAZ(myObs,myMotion,decision,command)-refAZ;
    while(deltaAZ > M_PI){
        deltaAZ-=2*M_PI;
    }
    while(deltaAZ <= -M_PI){
        deltaAZ+=2*M_PI;
    }
    return deltaAZ;
}

double getExpertDstPitch(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper_
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    asrc::core::util::AltitudeKeeper* altitudeKeeper;
    if(altitudeKeeper_){
        altitudeKeeper=altitudeKeeper_;
    }else{
        altitudeKeeper=new AltitudeKeeper();
    }

    MotionState originalMyMotion(myObs.at("motion")); // 元のparent座標系での値も必要

    std::string decisionType=decision["Vertical"][0];
    double value=decision["Vertical"][1];
    nl::json motionCmd=command.at("motion");

    if(decisionType=="El"){
        return value;
    }else if(decisionType=="Alt"){
        return altitudeKeeper->getDstPitch(myMotion,value);
    }else if(decisionType=="Pos"){
        // Deprecated
        return altitudeKeeper->getDstPitch(myMotion,-value);
    }else if(motionCmd.contains("dstDir")){
        Eigen::Vector3d dd=originalMyMotion.dirPtoH(motionCmd.at("dstDir"),originalMyMotion.pos(),"NED",false);
        return -atan2(dd(2),sqrt(dd(0)*dd(0)+dd(1)*dd(1)));
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDstPitch'.");
    }

    if(!altitudeKeeper_){
        delete altitudeKeeper;
    }
}

double getExpertDstAlt(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper_
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    asrc::core::util::AltitudeKeeper* altitudeKeeper;
    if(altitudeKeeper_){
        altitudeKeeper=altitudeKeeper_;
    }else{
        altitudeKeeper=new AltitudeKeeper();
    }

    MotionState originalMyMotion(myObs.at("motion")); // 元のparent座標系での値も必要

    std::string decisionType=decision["Vertical"][0];
    double value=decision["Vertical"][1];
    nl::json motionCmd=command.at("motion");

    if(decisionType=="El"){
        return altitudeKeeper->inverse(myMotion,value);
    }else if(decisionType=="Alt"){
        return value;
    }else if(decisionType=="Pos"){
        // Deprecated
        return -value;
    }else if(motionCmd.contains("dstDir")){
        Eigen::Vector3d dd=originalMyMotion.dirPtoH(motionCmd.at("dstDir"),originalMyMotion.pos(),"NED",false);
        double dstPitch_ex=-atan2(dd(2),sqrt(dd(0)*dd(0)+dd(1)*dd(1)));
        return altitudeKeeper->inverse(myMotion,dstPitch_ex);
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDstAlt'.");
    }

    if(!altitudeKeeper_){
        delete altitudeKeeper;
    }
}

double getExpertDeltaAlt(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    asrc::core::util::AltitudeKeeper* altitudeKeeper,
    const double& refAlt
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    return getExpertDstAlt(myObs,myMotion,decision,command,altitudeKeeper) - refAlt;
}

double getExpertDstV(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    double deltaV=getExpertDeltaV(
        myObs,
        myMotion,
        decision,
        command,
        minDeltaV,
        maxDeltaV,
        minDstAccel,
        maxDstAccel,
        minDstThrottle,
        maxDstThrottle,
        asTriChoice
    );
    return myMotion.vel.norm()+deltaV;
}

double getExpertDeltaV(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    std::string decisionType=decision["Throttle"][0];
    double value=decision["Throttle"][1];
    nl::json motionCmd=command.at("motion");
    double V=myMotion.vel.norm();

    if(maxDeltaV<=minDeltaV){
        throw std::runtime_error("maxDeltaV must be greater than minDeltaV.");
    }

    if(motionCmd.contains("dstV")){
        double dstV_ex=motionCmd.at("dstV");
        return std::clamp<double>(dstV_ex-V,minDeltaV,maxDeltaV);
    }else if(decisionType=="Vel"){
        return std::clamp<double>(value-V,minDeltaV,maxDeltaV);
    }else if(decisionType=="Throttle"){
        //0〜1のスロットルで指定していた場合
        if(maxDstThrottle<=minDstThrottle){
            throw std::runtime_error("maxDstThrottle must be greater than minDstThrottle.");
        }
        if(asTriChoice){
            if(value>maxDstThrottle){
                return maxDeltaV;
            }else if(value<minDstThrottle){
                return minDeltaV;
            }else{
                if(minDeltaV>0 || maxDeltaV<0){
                    return (minDeltaV+maxDeltaV)/2;
                }else{
                    return 0;
                }
            }
        }else{
            return minDeltaV+(maxDeltaV-minDeltaV)*std::clamp<double>(value,minDstThrottle,maxDstThrottle);
        }
    }else if(decisionType=="Accel"){
        //加速度ベースの指定だった場合
        if(maxDstAccel<=minDstAccel){
            throw std::runtime_error("maxDstAccel must be greater than minDstAccel.");
        }
        if(asTriChoice){
            if(value>maxDstAccel){
                return maxDeltaV;
            }else if(value<minDstAccel){
                return minDeltaV;
            }else{
                if(minDeltaV>0 || maxDeltaV<0){
                    return (minDeltaV+maxDeltaV)/2;
                }else{
                    return 0;
                }
            }
        }else{
            return minDeltaV+(maxDeltaV-minDeltaV)*std::clamp<double>(value,minDstAccel,maxDstAccel);
        }
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDeltaV'.");
    }
}

double getExpertDstThrottle(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    if(maxDstThrottle<=minDstThrottle){
        throw std::runtime_error("maxDstThrottle must be greater than minDstThrottle.");
    }

    std::string decisionType=decision["Throttle"][0];
    double value=decision["Throttle"][1];
    nl::json motionCmd=command.at("motion");
    double V=myMotion.vel.norm();

    if(motionCmd.contains("dstV")){
        if(maxDeltaV<=minDeltaV){
            throw std::runtime_error("maxDeltaV must be greater than minDeltaV.");
        }

        double dstV_ex=motionCmd.at("dstV");
        if(asTriChoice){
            if(dstV_ex-V>maxDeltaV){
                return maxDstThrottle;
            }else if(dstV_ex-V<minDeltaV){
                return minDstThrottle;
            }else{
                return (minDstThrottle+maxDstThrottle)/2;
            }
        }else{
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*std::clamp<double>(dstV_ex-V,minDeltaV,maxDeltaV);
        }
    }else if(decisionType=="Vel"){
        if(maxDeltaV<=minDeltaV){
            throw std::runtime_error("maxDeltaV must be greater than minDeltaV.");
        }

        if(asTriChoice){
            if(value-V>maxDeltaV){
                return maxDstThrottle;
            }else if(value-V<minDeltaV){
                return minDstThrottle;
            }else{
                return (minDstThrottle+maxDstThrottle)/2;
            }
        }else{
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*std::clamp<double>(value-V,minDeltaV,maxDeltaV);
        }
    }else if(decisionType=="Throttle"){
        //0〜1のスロットルで指定していた場合
        return std::clamp<double>(value,minDstThrottle,maxDstThrottle);
    }else if(decisionType=="Accel"){
        //加速度ベースの指定だった場合
        if(maxDstAccel<=minDstAccel){
            throw std::runtime_error("maxDstAccel must be greater than minDstAccel.");
        }
        if(asTriChoice){
            if(value>maxDstAccel){
                return maxDstThrottle;
            }else if(value<minDstAccel){
                return minDstThrottle;
            }else{
                return (minDstThrottle+maxDstThrottle)/2;
            }
        }else{
            return minDstThrottle+(maxDstThrottle-minDstThrottle)*std::clamp<double>(value,minDstAccel,maxDstAccel);
        }
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDeltaV'.");
    }
}

double getExpertDstAccel(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const double& minDeltaV,
    const double& maxDeltaV,
    const double& minDstAccel,
    const double& maxDstAccel,
    const double& minDstThrottle,
    const double& maxDstThrottle,
    bool asTriChoice
){
    if(!myObs.at("isAlive")){
        return 0.0;
    }

    if(maxDstAccel<=minDstAccel){
        throw std::runtime_error("maxDstAccel must be greater than minDstAccel.");
    }

    std::string decisionType=decision["Throttle"][0];
    double value=decision["Throttle"][1];
    nl::json motionCmd=command.at("motion");
    double V=myMotion.vel.norm();

    if(motionCmd.contains("dstV")){
        if(maxDeltaV<=minDeltaV){
            throw std::runtime_error("maxDeltaV must be greater than minDeltaV.");
        }

        double dstV_ex=motionCmd.at("dstV");
        if(asTriChoice){
            if(dstV_ex-V>maxDeltaV){
                return maxDstAccel;
            }else if(dstV_ex-V<minDeltaV){
                return minDstAccel;
            }else{
                if(minDstAccel>0 || maxDstAccel<0){
                    return (minDstAccel+maxDstAccel)/2;
                }else{
                    return 0;
                }
            }
        }else{
            return minDstAccel+(maxDstAccel-minDstAccel)*std::clamp<double>(dstV_ex-V,minDeltaV,maxDeltaV);
        }
    }else if(decisionType=="Vel"){
        if(maxDeltaV<=minDeltaV){
            throw std::runtime_error("maxDeltaV must be greater than minDeltaV.");
        }

        if(asTriChoice){
            if(value-V>maxDeltaV){
                return maxDstAccel;
            }else if(value-V<minDeltaV){
                return minDstAccel;
            }else{
                if(minDstAccel>0 || maxDstAccel<0){
                    return (minDstAccel+maxDstAccel)/2;
                }else{
                    return 0;
                }
            }
        }else{
            return minDstAccel+(maxDstAccel-minDstAccel)*std::clamp<double>(value-V,minDeltaV,maxDeltaV);
        }
    }else if(decisionType=="Throttle"){
        //0〜1のスロットルで指定していた場合
        if(maxDstThrottle<=minDstThrottle){
            throw std::runtime_error("maxDstThrottle must be greater than minDstThrottle.");
        }

        if(asTriChoice){
            if(value>maxDstThrottle){
                return maxDstAccel;
            }else if(value<minDstThrottle){
                return minDstAccel;
            }else{
                if(minDstAccel>0 || maxDstAccel<0){
                    return (minDstAccel+maxDstAccel)/2;
                }else{
                    return 0;
                }
            }
        }else{
            return minDstAccel+(maxDstAccel-minDstAccel)*std::clamp<double>(value,minDstThrottle,maxDstThrottle);
        }
    }else if(decisionType=="Accel"){
        //加速度ベースの指定だった場合
        return std::clamp<double>(value,minDstAccel,maxDstAccel);
    }else{
        throw std::runtime_error("Given unsupported expert's decision & command for the utility function 'getExpertDeltaV'.");
    }
}

int getExpertShotTarget(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const std::vector<asrc::core::Track3D>& tracks
){
    if(!myObs.at("isAlive")){
        return -1;
    }

    if(decision["Fire"][0]){
        Track3D expertTarget=decision["Fire"][1];

        for(int tIdx=0;tIdx<tracks.size();++tIdx){
            const auto& t=tracks[tIdx];
            if(t.isSame(expertTarget)){
                return tIdx;
            }
        }
    }
    return -1;
}

int getExpertShotTarget(
    const nl::json& myObs,
    const asrc::core::MotionState& myMotion,
    const nl::json& decision,
    const nl::json& command,
    const py::list& tracks
){
    if(!myObs.at("isAlive")){
        return -1;
    }

    for(int tIdx=0;tIdx<py::len(tracks);++tIdx){
        Track3D expertTarget=decision["Fire"][1];

        const Track3D& t=tracks[tIdx].cast<std::reference_wrapper<Track3D>>().get();
        if(t.isSame(expertTarget)){
            return tIdx;
        }
    }
    return  -1;
}

void exportConvertActionFromAnotherAgent(py::module &m)
{
    using namespace pybind11::literals;

    m.def("getExpertDstAZ",&getExpertDstAZ);
    m.def("getExpertDeltaAZ",&getExpertDeltaAZ);

    m.def("getExpertDstPitch",&getExpertDstPitch);
    m.def("getExpertDstAlt",&getExpertDstAlt);
    m.def("getExpertDeltaAlt",&getExpertDeltaAlt);

    m.def("getExpertDstV",&getExpertDstV);
    m.def("getExpertDeltaV",&getExpertDeltaV);
    m.def("getExpertDstThrottle",&getExpertDstThrottle);
    m.def("getExpertDstAccel",&getExpertDstAccel);

    m.def("getExpertShotTarget",py::overload_cast<const nl::json&,const asrc::core::MotionState&,const nl::json&,const nl::json&,const std::vector<asrc::core::Track3D>&>(&getExpertShotTarget));
    m.def("getExpertShotTarget",py::overload_cast<const nl::json&,const asrc::core::MotionState&,const nl::json&,const nl::json&,const py::list&>(&getExpertShotTarget));
}

    }
}

ASRC_PLUGIN_NAMESPACE_END
