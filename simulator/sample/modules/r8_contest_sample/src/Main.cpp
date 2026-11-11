// Copyright (c) 2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include <ASRCAISim1/Factory.h>
#include "Common.h"
#include "VirtualSimulatorSample.h"
#include "R8ContestAgentSample01.h"
#include "R8ContestRewardSample01.h"
#include "R8ContestRewardSample02.h"

ASRC_PLUGIN_NAMESPACE_BEGIN

ASRC_EXPORT_PYTHON_MODULE(m,factoryHelper)
    bindSTLContainer(m); // py::module_local(true)なSTLコンテナのバインドを行う。

	exportVirtualSimulatorSample(m);
    exportR8ContestAgentSample01(m,factoryHelper);
    exportR8ContestRewardSample01(m,factoryHelper);
    exportR8ContestRewardSample02(m,factoryHelper);

}

ASRC_PLUGIN_NAMESPACE_END
