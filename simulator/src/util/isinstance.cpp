// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "util/isinstance.h"
#include "Factory.h"
#include "Entity.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
ASRC_INLINE_NAMESPACE_BEGIN(util)

bool isinstance(const std::shared_ptr<const Entity>& instance, const std::string& className){
    return Factory::isinstance(instance,className);
}

bool isinstance(const std::shared_ptr<const Entity>& instance,const std::string& baseName,const std::string& className){
    return Factory::isinstance(instance,baseName,className);
}

void exportIsinstance(py::module &m){
    m.def("isinstance_name",static_cast<bool(*)(const std::shared_ptr<const Entity>&,const std::string&)>(&asrc::core::util::isinstance));
    m.def("isinstance_name",static_cast<bool(*)(const std::shared_ptr<const Entity>&,const std::string&,const std::string&)>(&asrc::core::util::isinstance));
}

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
