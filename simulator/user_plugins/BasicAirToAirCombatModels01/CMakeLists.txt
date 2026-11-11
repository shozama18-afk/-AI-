# Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#----------------------------------------------------------------
# CMakeLists.txt template for ASRCAISim1's user_plugin
#----------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)
cmake_policy(PUSH)
cmake_policy(VERSION 3.10...3.29)
project(${TARGET_NAME}${BUILD_IDENTIFIER})
if(${WITH_THIRDPARTIES})
    if(EXISTS "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/thirdPartyConfigPath.cmake")
        include("${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/thirdPartyConfigPath.cmake")
    endif()
endif()
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/${TARGET_NAME}")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
#----------------------------------------------------------------
# find packages
#----------------------------------------------------------------
list(GET ASRC_PATH_INFO 0 ASRC_Python3_INCLUDE_DIRS)
list(GET ASRC_PATH_INFO 1 ASRC_Python3_LIBRARY_DIRS)
list(GET ASRC_PATH_INFO 2 ASRC_Python3_Numpy_INCLUDE_DIRS)
list(GET ASRC_PATH_INFO 3 ASRC_CORE_SIMULATOR_DIR)
set(PythonLibsNew_FIND_VERSION $ENV{PythonLibsNew_FIND_VERSION})
set(Python_ROOT_DIR "$ENV{Python_ROOT_DIR}")
set(Python3_ROOT_DIR "$ENV{Python3_ROOT_DIR}")
find_package(ASRCAISim1 REQUIRED) # put 1st
find_package(Python $ENV{PythonLibsNew_FIND_VERSION} COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
message(STATUS "Python Executable:${Python3_EXECUTABLE}")
message(STATUS "Python Include:${Python3_INCLUDE_DIRS}")
message(STATUS "Python Libs:${Python3_LIBRARY_DIRS}")
message(STATUS "Numpy:${Python3_Numpy_INCLUDE_DIRS}")
message(STATUS "Pybind11:${pybind11_INCLUDE_DIR}")
message(STATUS "Core Simulator Dir:${ASRCAISim1_DIR}")

# include ASRC's CMake utility functions
find_file(path_asrc_cmake_utility "asrc_cmake_utility.cmake")
include("${path_asrc_cmake_utility}")

#----------------------------------------------------------------
# target configuration
#----------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE True)
if(NOT ${MSYS_ROOT_DIR} STREQUAL "")
    #MSYS2 seems it does not add /usr/local by default.
    include_directories(
        "${MSYS_ROOT_DIR}usr/local/include"
    )
    link_directories(
        "${MSYS_ROOT_DIR}usr/local/lib"
    )
endif()

file(GLOB_RECURSE source "${PROJECT_SOURCE_DIR}/src/**.cpp" "${PROJECT_SOURCE_DIR}/src/**.c")
pybind11_add_module(${CMAKE_PROJECT_NAME} SHARED
    ${source}
)

asrc_set_common_cxx_flags()
asrc_set_common_compile_definitions(${TARGET_NAME} "${BUILD_IDENTIFIER}")
asrc_set_common_compile_options(${TARGET_NAME} "${BUILD_IDENTIFIER}")
asrc_set_common_properties(${TARGET_NAME} "${BUILD_IDENTIFIER}")
asrc_set_common_include_directories(${TARGET_NAME} "${BUILD_IDENTIFIER}")

#----------------------------------------------------------------
# link libraries
#----------------------------------------------------------------
asrc_common_link_libraries(${TARGET_NAME} "${BUILD_IDENTIFIER}")

#----------------------------------------------------------------
# installation after build
#----------------------------------------------------------------
asrc_install_user_plugin("${TARGET_NAME}" "${BUILD_IDENTIFIER}")

#----------------------------------------------------------------
# add user-defined dependencies
#----------------------------------------------------------------
#find_package(a_third_party_package REQUIRED)
#target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
#    a_third_party_package::target
#)

cmake_policy(POP)
