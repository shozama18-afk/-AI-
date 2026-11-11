# Copyright (c) 2021-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
cmake_minimum_required(VERSION 3.10)
cmake_policy(PUSH)
cmake_policy(VERSION 3.10...3.29)
project(${TARGET_NAME})
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
set(PythonLibsNew_FIND_VERSION $ENV{PythonLibsNew_FIND_VERSION})
set(Python_ROOT_DIR "$ENV{Python_ROOT_DIR}")
set(Python3_ROOT_DIR "$ENV{Python3_ROOT_DIR}")
find_package(Python $ENV{PythonLibsNew_FIND_VERSION} COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(NLopt REQUIRED)
find_package(Boost REQUIRED)
message(STATUS "Python Executable:${Python3_EXECUTABLE}")
message(STATUS "Python Include:${Python3_INCLUDE_DIRS}")
message(STATUS "Python Libs:${Python3_LIBRARY_DIRS}")
message(STATUS "Numpy:${Python3_Numpy_INCLUDE_DIRS}")

# include ASRC's CMake utility functions
include("${PROJECT_SOURCE_DIR}/${TARGET_NAME}/share/cmake/asrc_cmake_utility.cmake")

#----------------------------------------------------------------
# target configuration
#----------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE True)
if(NOT "${MSYS_ROOT_DIR}" STREQUAL "")
    #MSYS2 seems it does not add /usr/local by default.
    include_directories(
        "${MSYS_ROOT_DIR}usr/local/include"
    )
    link_directories(
        "${MSYS_ROOT_DIR}usr/local/lib"
    )
endif()

file(GLOB_RECURSE source "${PROJECT_SOURCE_DIR}/src/**.cpp" "${PROJECT_SOURCE_DIR}/src/**.c")
pybind11_add_module(Core SHARED THIN_LTO
    ${source}
)

asrc_set_common_cxx_flags()
asrc_set_common_compile_definitions(Core "")
asrc_set_common_compile_options(Core "")
set(ASRC_INSTALL_RPATH_RELATIVE_TO_ORIGIN "../thirdParty/lib")
asrc_set_common_properties(Core "")
asrc_set_common_include_directories(Core "")

target_include_directories(Core PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/include/eigen3>"
    "$<INSTALL_INTERFACE:thirdParty/include/eigen3>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/include/eigen3/unsupported>"
    "$<INSTALL_INTERFACE:thirdParty/include/eigen3/unsupported>"
)
target_link_libraries(Core
    PUBLIC
        NLopt::nlopt
        Boost::headers
)
if(WIN32)
    target_link_libraries(Core PUBLIC
        bcrypt
    )
else()
    target_link_libraries(Core PUBLIC
        m
    )
endif()

#----------------------------------------------------------------
# installation after build
#----------------------------------------------------------------
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/share/cmake/${CMAKE_PROJECT_NAME}Config.cmake.in"
    "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${CMAKE_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION share/cmake
)
install(
    FILES "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${CMAKE_PROJECT_NAME}Config.cmake"
    DESTINATION share/cmake
)

install(
    TARGETS Core
    EXPORT Core
    DESTINATION core
)
install(
    EXPORT Core
    NAMESPACE ASRCAISim1::
    FILE ASRCAISim1Targets.cmake
    DESTINATION share/cmake
)
if(EXISTS "${PROJECT_SOURCE_DIR}/include")
    install(
        DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "include/${TARGET_NAME}"
    )
endif()
if(${WITH_THIRDPARTIES})
    install(
        DIRECTORY "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/"
        DESTINATION "thirdParty"
    )
endif()

cmake_policy(POP)
