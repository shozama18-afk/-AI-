// Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include <memory>
#include <vector>
#include <map>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include "MathUtility.h"
#include "PhysicalAsset.h"
#include "Track.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(Sensor,PhysicalAsset)
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
};
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(Sensor3D,Sensor)
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual std::pair<bool,std::vector<Track3D>> isTrackingAny();
    virtual std::pair<bool,Track3D> isTracking(const std::weak_ptr<PhysicalAsset>& target_);
    virtual std::pair<bool,Track3D> isTracking(const Track3D& target_);
    virtual std::pair<bool,Track3D> isTracking(const boost::uuids::uuid& target_);
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(Sensor3D)
    virtual std::pair<bool,std::vector<Track3D>> isTrackingAny() override{
        typedef  std::pair<bool,std::vector<Track3D>> retType;
        PYBIND11_OVERRIDE(retType,Base,isTrackingAny);
    }
    virtual std::pair<bool,Track3D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override{
        typedef  std::pair<bool,Track3D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
    virtual std::pair<bool,Track3D> isTracking(const Track3D& target_) override{
        typedef  std::pair<bool,Track3D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
    virtual std::pair<bool,Track3D> isTracking(const boost::uuids::uuid& target_) override{
        typedef  std::pair<bool,Track3D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
};
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(Sensor2D,Sensor)
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual std::pair<bool,std::vector<Track2D>> isTrackingAny();
    virtual std::pair<bool,Track2D> isTracking(const std::weak_ptr<PhysicalAsset>& target_);
    virtual std::pair<bool,Track2D> isTracking(const Track2D& target_);
    virtual std::pair<bool,Track2D> isTracking(const boost::uuids::uuid& target_);
};
ASRC_DECLARE_DERIVED_TRAMPOLINE(Sensor2D)
    virtual std::pair<bool,std::vector<Track2D>> isTrackingAny() override{
        typedef  std::pair<bool,std::vector<Track2D>> retType;
        PYBIND11_OVERRIDE(retType,Base,isTrackingAny);
    }
    virtual std::pair<bool,Track2D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override{
        typedef  std::pair<bool,Track2D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
    virtual std::pair<bool,Track2D> isTracking(const Track2D& target_) override{
        typedef  std::pair<bool,Track2D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
    virtual std::pair<bool,Track2D> isTracking(const boost::uuids::uuid& target_) override{
        typedef  std::pair<bool,Track2D> retType;
        PYBIND11_OVERRIDE(retType,Base,isTracking,target_);
    }
};


ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(AircraftRadar,Sensor3D)
    public:
    //parameters
    double Lref,thetaFOR;
    //internal variables
    std::vector<Track3D> track;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void setDependency() override;
    virtual void perceive(bool inReset) override;
    virtual std::pair<bool,std::vector<Track3D>> isTrackingAny() override;
    virtual std::pair<bool,Track3D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override;
    virtual std::pair<bool,Track3D> isTracking(const Track3D& target_) override;
    virtual std::pair<bool,Track3D> isTracking(const boost::uuids::uuid& target_) override;
};

/**
 * @class IRST
 * @brief IRセンサ(Infra-Red Search and Track)を表すクラス
 * 
 * @details
 *      最も基本的なIRSTモデル。
 *      目標の方向(と方向変化率)のみをTrack2Dとして観測できる。
 * 
 *      複数の戦闘機が生成したTrack2Dのtruthを照合してペアを特定して三角測量を行なうことを防ぐため、
 *      truthにはランダムなUUIDを入れる。
 * 
 *      - 探知誤差を無視し、真の値を航跡化する。
 *      - ジンバル等の首振りを無視し、瞬時視野角を考慮しない。
 * 
 *      赤外放射強度(irRadiationIntensityScale)が1の目標に対する基準探知距離をパラメータとして、
 *      赤外放射強度の平方根に比例する最大探知距離内に目標がいる場合に探知できるものとする。
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(IRST,Sensor2D)
    public:
    //parameters
    double Lref; //!< 赤外放射強度(irRadiationIntensityScale)が1の目標に対する基準探知距離[m]
    double thetaFOR; //!< 覆域の角度(Field Of Regard)をdegで指定する
    //internal variables
    std::vector<Track2D> track;
    std::map<boost::uuids::uuid, boost::uuids::uuid> trackUUIDs; //!< 探知したEntityのuuidForTrackからIRST航跡用の匿名UUIDへのマッピング。直前の観測ステップで探知できていたもののみ保持する。
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void setDependency() override;
    virtual void perceive(bool inReset) override;
    virtual std::pair<bool,std::vector<Track2D>> isTrackingAny() override;
    virtual std::pair<bool,Track2D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override;
    virtual std::pair<bool,Track2D> isTracking(const Track2D& target_) override;
    virtual std::pair<bool,Track2D> isTracking(const boost::uuids::uuid& target_) override;
};

ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(MWS,Sensor2D)
    public:
    //parameters
    bool isEsmIsh;
    double Lref,thetaFOR;
    //internal variables
    std::vector<Track2D> track;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void setDependency() override;
    virtual void perceive(bool inReset) override;
    virtual std::pair<bool,std::vector<Track2D>> isTrackingAny() override;
    virtual std::pair<bool,Track2D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override;
    virtual std::pair<bool,Track2D> isTracking(const Track2D& target_) override;
    virtual std::pair<bool,Track2D> isTracking(const boost::uuids::uuid& target_) override;
};

ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(MissileSensor,Sensor3D)
    public:
    //parameters
    double Lref,thetaFOR,thetaFOV;
    //internal variables
    std::vector<Track3D> track;
    Track3D target;
    Coordinate estTPos;
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    virtual void setDependency() override;
    virtual void perceive(bool inReset) override;
    virtual void control() override;
    virtual void kill() override;
    virtual std::pair<bool,std::vector<Track3D>> isTrackingAny() override;
    virtual std::pair<bool,Track3D> isTracking(const std::weak_ptr<PhysicalAsset>& target_) override;
    virtual std::pair<bool,Track3D> isTracking(const Track3D& target_) override;
    virtual std::pair<bool,Track3D> isTracking(const boost::uuids::uuid& target_) override;
};

void exportSensor(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
