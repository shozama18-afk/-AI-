// Copyright (c) 2026-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "util/imitation/init.h"

ASRC_PLUGIN_NAMESPACE_BEGIN

namespace util{
	namespace imitation{

void init(py::module &m_parent)
{
	using namespace pybind11::literals;
	auto m=m_parent.def_submodule("imitation");
	exportConvertActionFromAnotherAgent(m);
}

	}
}

ASRC_PLUGIN_NAMESPACE_END
