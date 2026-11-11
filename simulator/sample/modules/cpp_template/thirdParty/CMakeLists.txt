# Copyright (c) 2025-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#----------------------------------------------------------------
# CMakeLists.txt wrapper template for building a third-party dependency with a specific ASRCAISim1 installation environment.
#
# This wrapper resolves the paths of third-party libraries depended on by ASRCAISim1
# and the plugins which is depended on by this plugin in the current Python environment,
# so that this plugin can find the correct third-party libraries installed with ASRCAISim1 and other plugins.
#----------------------------------------------------------------

#----------------------------------------------------------------
# find python
#----------------------------------------------------------------
set(PythonLibsNew_FIND_VERSION $ENV{PythonLibsNew_FIND_VERSION})
set(Python_ROOT_DIR "$ENV{Python_ROOT_DIR}")
set(Python3_ROOT_DIR "$ENV{Python3_ROOT_DIR}")

#----------------------------------------------------------------
# find core simulator and utility
#----------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH $ENV{ASRC_CMAKE_PREFIX_PATH})
find_package(ASRCAISim1 REQUIRED)

# include ASRC's CMake utility functions
find_file(path_asrc_cmake_utility "asrc_cmake_utility.cmake")
include("${path_asrc_cmake_utility}")

#----------------------------------------------------------------
# find plugins
#----------------------------------------------------------------
asrc_find_depending_plugins(
    "${Python3_EXECUTABLE}"
    "$ENV{PROJECT_ROOT_DIR}/$ENV{ASRC_PLUGIN_NAME}/PluginDependencyInfo.json"
    "$ENV{ASRC_PLUGIN_NAME}$ENV{ASRC_BUILD_IDENTIFIER}"
    "$ENV{ASRC_BUILD_IDENTIFIER}"
)

message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")

add_subdirectory(${THIRDPARTY_SOURCE_DIR} ${CMAKE_BINARY_DIR})
