#
# 依存プラグインをfind_packageで検索してターゲットを追加する。
#
# 引数
# Python3_EXECUTABLE : Python3の実行ファイルのパス。
# DEPENDENCY_INFO_JSON_FILE : PluginDependencyInfo.jsonのパス。
#
function(asrc_find_depending_plugins Python3_EXECUTABLE DEPENDENCY_INFO_JSON_FILE)

    # 直接依存するプラグインの一覧を取得する。
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "-m" "ASRCAISim1.scripts.find_plugin_dependencies" "${DEPENDENCY_INFO_JSON_FILE}"
        COMMAND "tail" "-n1"
        OUTPUT_VARIABLE _DEPENDING_PLUGIN_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # 未読み込みなら読み込む
    foreach(_PLUGIN_DIR IN LISTS _DEPENDING_PLUGIN_DIRS)
        get_filename_component(_PLUGIN_NAME "${_PLUGIN_DIR}" NAME_WE)
        message(STATUS "Search ${_PLUGIN_DIR}/share/cmake")
        if(NOT TARGET ${_PLUGIN_NAME}::All)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${_PLUGIN_DIR}/share/cmake")
            #set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
            set(PACKAGE_NAME_PREV ${PACKAGE_NAME})
            find_package(${_PLUGIN_NAME})
            set(PACKAGE_NAME ${PACKAGE_NAME_PREV})
        endif()
    endforeach()

endfunction()

#
# 依存プラグインをfind_packageで検索してターゲットを追加し、指定したターゲットにリンクする。
#
# 引数
# Python3_EXECUTABLE : Python3の実行ファイルのパス。
# DEPENDENCY_INFO_JSON_FILE : PluginDependencyInfo.jsonのパス。
# DEPENDENT_TARGET : リンク先のターゲット
# BUILD_IDENTIFIER : Build Identifierありでビルドするかどうか。
#
function(asrc_find_and_link_depending_plugins Python3_EXECUTABLE DEPENDENCY_INFO_JSON_FILE DEPENDENT_TARGET BUILD_IDENTIFIER)
    if(NOT TARGET ${DEPENDENT_TARGET})
        message(FATAL_ERROR "The target \"${DEPENDENT_TARGET}\" does not exist.")
    endif()

    # 直接依存するプラグインの一覧を取得する。
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "-m" "ASRCAISim1.scripts.find_plugin_dependencies" "${DEPENDENCY_INFO_JSON_FILE}"
        COMMAND "tail" "-n1"
        OUTPUT_VARIABLE _DEPENDING_PLUGIN_DIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # 未読み込みなら読み込む
    foreach(_PLUGIN_DIR IN LISTS _DEPENDING_PLUGIN_DIRS)
        get_filename_component(_PLUGIN_NAME "${_PLUGIN_DIR}" NAME_WE)
        message(STATUS "Search ${_PLUGIN_DIR}/share/cmake")
        if(NOT TARGET ${_PLUGIN_NAME}::All)
            set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${_PLUGIN_DIR}/share/cmake")
            #set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
            set(ASRC_PACKAGE_NAME_PREV ${PACKAGE_NAME})
            find_package(${_PLUGIN_NAME})
            set(PACKAGE_NAME ${ASRC_PACKAGE_NAME_PREV})
        endif()

        if(TARGET ${_PLUGIN_NAME}::All)
            if(NOT "${BUILD_IDENTIFIER}" STREQUAL "")
                target_link_libraries(${DEPENDENT_TARGET} PUBLIC
                    "${_PLUGIN_NAME}::All_B"
                )
            else()
                target_link_libraries(${DEPENDENT_TARGET} PUBLIC
                    "${_PLUGIN_NAME}::All"
                )
            endif()
        endif()
    endforeach()

endfunction()

function(asrc_set_common_cxx_flags)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -rdynamic" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2" PARENT_SCOPE)
endfunction()

function(asrc_set_common_compile_definitions PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")

    target_compile_definitions(${TARGET_NAME}
        PUBLIC
            EIGEN_MPL2_ONLY
            _USE_MATH_DEFINES
            HAVE_SNPRINTF
        PRIVATE
            MODULE_NAME=${PLUGIN_NAME}
    )
    if(MSVC)
        target_compile_definitions(${TARGET_NAME}
            PUBLIC
                NOMINMAX
        )
    endif()

    if(NOT "${BUILD_IDENTIFIER}" STREQUAL "")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                MODULE_BUILD_IDENTIFIER=${BUILD_IDENTIFIER}
        )
    else()
        # the parts independent of build identifier should be built iff built without build identifier
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                ASRC_BUILD_PLUGIN_BII_PART
        )
    endif()
endfunction()

function(asrc_set_common_compile_options PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")
    if(MSVC)
        target_compile_options(${TARGET_NAME}
            PUBLIC
                "/utf-8"
                "/bigobj"
                "/D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING"
        )
    endif()
endfunction()

function(asrc_set_common_properties PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")
    if(WIN32)
        if(MSVC)
            set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".dll")
        else()
            set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "lib" SUFFIX ".dll")
        endif()
    else()
        set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "lib" SUFFIX ".so")
    endif()
    if(NOT ASRC_INSTALL_RPATH_RELATIVE_TO_ORIGIN)
        set(ASRC_INSTALL_RPATH_RELATIVE_TO_ORIGIN "thirdParty/lib")
    endif()
    if(${WITH_THIRDPARTIES})
        if(NOT WIN32)
            if(APPLE)
                set_target_properties(${TARGET_NAME} PROPERTIES
                    BUILD_WITH_INSTALL_RPATH FALSE
                    INSTALL_RPATH "@loader_path/${ASRC_INSTALL_RPATH_RELATIVE_TO_ORIGIN}"
                )
            else()
                set_target_properties(${TARGET_NAME} PROPERTIES
                    BUILD_WITH_INSTALL_RPATH FALSE
                    INSTALL_RPATH "$ORIGIN/${ASRC_INSTALL_RPATH_RELATIVE_TO_ORIGIN}"
                )
            endif()
        endif()
    endif()
endfunction()

function(asrc_set_common_include_directories PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")
    target_include_directories(${TARGET_NAME}
        PUBLIC
            "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:include>"
    )
    if(${WITH_THIRDPARTIES})
        target_include_directories(${TARGET_NAME}
            PUBLIC
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/include>"
                "$<INSTALL_INTERFACE:thirdParty/include>"
        )
    endif()
endfunction()

function(asrc_common_link_libraries PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")
    target_link_libraries(${TARGET_NAME} PUBLIC
        ASRCAISim1::Core
    )

    # 依存するASRCシミュレータプラグインの探索
    asrc_find_and_link_depending_plugins(
        "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/${PLUGIN_NAME}/PluginDependencyInfo.json"
        "${TARGET_NAME}"
        "${BUILD_IDENTIFIER}"
    )

    if(NOT "${BUILD_IDENTIFIER}" STREQUAL "")
        target_link_directories(${TARGET_NAME}
            PRIVATE
                "${PROJECT_SOURCE_DIR}/${PLUGIN_NAME}"
        )
        target_link_libraries(${TARGET_NAME} PRIVATE
            ${PLUGIN_NAME}
        )
    endif()
endfunction()

function(asrc_install_core_plugin PLUGIN_NAME)
    set(TARGET_NAME "${PLUGIN_NAME}")
    if("${BUILD_IDENTIFIER}" STREQUAL "")
        include(CMakePackageConfigHelpers)
        configure_package_config_file(
            "${ASRCAISim1_DIR}/share/cmake/PluginTemplate.cmake.in"
            "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${TARGET_NAME}Config.cmake"
            INSTALL_DESTINATION share/cmake
        )
        install(
            FILES "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${TARGET_NAME}Config.cmake"
            DESTINATION share/cmake
        )
    endif()

    install(
        TARGETS "${TARGET_NAME}"
        EXPORT "${TARGET_NAME}"
        DESTINATION .
    )
    install(
        EXPORT "${TARGET_NAME}"
        NAMESPACE "${PLUGIN_NAME}::"
        FILE "${TARGET_NAME}Targets.cmake"
        DESTINATION share/cmake
    )
    if(EXISTS "${PROJECT_SOURCE_DIR}/include")
        install(
            DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
            DESTINATION "include/ASRCAISim1"
        )
    endif()
    
    if(${WITH_THIRDPARTIES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/")
            install(
                DIRECTORY "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/"
                DESTINATION "thirdParty"
            )
        endif()
    endif()
endfunction()

function(asrc_install_user_plugin PLUGIN_NAME BUILD_IDENTIFIER)
    set(TARGET_NAME "${PLUGIN_NAME}${BUILD_IDENTIFIER}")
    if("${BUILD_IDENTIFIER}" STREQUAL "")
        include(CMakePackageConfigHelpers)
        configure_package_config_file(
            "${ASRCAISim1_DIR}/share/cmake/PluginTemplate.cmake.in"
            "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${TARGET_NAME}Config.cmake"
            INSTALL_DESTINATION share/cmake
        )
        install(
            FILES "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/${TARGET_NAME}Config.cmake"
            DESTINATION share/cmake
        )
    endif()

    install(
        TARGETS "${TARGET_NAME}"
        EXPORT "${TARGET_NAME}"
        DESTINATION .
    )
    install(
        EXPORT "${TARGET_NAME}"
        NAMESPACE "${PLUGIN_NAME}::"
        FILE "${TARGET_NAME}Targets.cmake"
        DESTINATION share/cmake
    )
    if(EXISTS "${PROJECT_SOURCE_DIR}/include")
        if(NOT "${BUILD_IDENTIFIER}" STREQUAL "")
            install(
                DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
                DESTINATION "include/${BUILD_IDENTIFIER}/${PLUGIN_NAME}"
            )
        else()
            install(
                DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
                DESTINATION "include/${PLUGIN_NAME}"
            )
        endif()
    endif()
    
    if(${WITH_THIRDPARTIES})
        if(EXISTS "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/")
            install(
                DIRECTORY "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/thirdParty/install/"
                DESTINATION "thirdParty"
            )
        endif()
    endif()
endfunction()
