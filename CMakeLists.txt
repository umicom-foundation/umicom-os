#-----------------------------------------------------------------------------
# Umicom OS
# File: CMakeLists.txt
#
# PURPOSE:
#   Build the Umicom OS user-space application against the root-level Umicom
#   Framework submodule while keeping Linux kernel/bootstrap work independent.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomOS LANGUAGES C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)
option(UMICOM_OS_GUI "Build the GTK4 Umicom OS desktop shell" OFF)
option(UMICOM_OS_BUILD_TESTS "Build Umicom OS tests" ON)
# Source and installed SDK dependency modes are explicit. Retain the former
# unconditional source setup for review; the default mode preserves that build.
if(FALSE)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/framework/CMakeLists.txt")
    message(FATAL_ERROR "Umicom Framework submodule is missing. Run: git submodule update --init --recursive")
endif()
if(UMICOM_OS_GUI)
    set(UMICOM_BUILD_GTK4_ADAPTER ON CACHE BOOL "Build Framework GTK4 adapter" FORCE)
endif()
add_subdirectory(framework)
endif()

option(UMICOM_OS_USE_INSTALLED_FRAMEWORK "Build OS user space with an explicitly selected installed Framework SDK" OFF)
set(UMICOM_SDK_ROOT "" CACHE PATH "Selected installed Framework SDK prefix")
if(UMICOM_OS_USE_INSTALLED_FRAMEWORK)
    if(NOT IS_ABSOLUTE "${UMICOM_SDK_ROOT}" OR NOT IS_DIRECTORY "${UMICOM_SDK_ROOT}")
        message(FATAL_ERROR "Installed Framework mode requires an absolute existing UMICOM_SDK_ROOT")
    endif()
    set(_umicom_os_components base runtime platform security ui ui_components)
    if(UMICOM_OS_GUI)
        list(APPEND _umicom_os_components ui_gtk4 desktop_workspace_gtk4 desktop_system_gtk4)
    endif()
    find_package(UmicomFramework 0.9 CONFIG REQUIRED COMPONENTS ${_umicom_os_components}
        PATHS "${UMICOM_SDK_ROOT}" NO_DEFAULT_PATH)
    foreach(_helper umicom_require_installed_sdk umicom_require_sdk_environment umicom_apply_warnings umicom_apply_sanitizers)
        if(NOT COMMAND ${_helper})
            message(FATAL_ERROR "Selected Framework SDK lacks required consumer helper: ${_helper}")
        endif()
    endforeach()
    set(_umicom_os_targets)
    foreach(_component IN LISTS _umicom_os_components)
        list(APPEND _umicom_os_targets "Umicom::${_component}")
    endforeach()
    umicom_require_installed_sdk("${UMICOM_SDK_ROOT}" ${_umicom_os_targets})
    umicom_require_sdk_environment(2)
else()
    if(UMICOM_SDK_ROOT)
        message(FATAL_ERROR "Set UMICOM_OS_USE_INSTALLED_FRAMEWORK=ON to use UMICOM_SDK_ROOT; use a separate build directory")
    endif()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/framework/CMakeLists.txt")
        message(FATAL_ERROR "Framework source is missing. Supply the reviewed submodule or select an installed SDK with UMICOM_OS_USE_INSTALLED_FRAMEWORK=ON")
    endif()
    if(UMICOM_OS_GUI)
        set(UMICOM_BUILD_GTK4_ADAPTER ON CACHE BOOL "Build Framework GTK4 adapter" FORCE)
    endif()
    add_subdirectory(framework)
endif()
add_subdirectory(applications/os)
