#-----------------------------------------------------------------------------
# Umicom OS
# File: CMakeLists.txt
#
# PURPOSE:
#   Build the Umicom OS user-space application against the root-level Umicom
#   Framework submodule while keeping Linux kernel/bootstrap work independent.
#
# Created by: Sammy Hegab
# Organisation: Umicom Foundation
# Licence: MIT
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomOS LANGUAGES C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)
option(UMICOM_OS_GUI "Build the GTK4 Umicom OS desktop shell" OFF)
option(UMICOM_OS_BUILD_TESTS "Build Umicom OS tests" ON)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/framework/CMakeLists.txt")
    message(FATAL_ERROR "Umicom Framework submodule is missing. Run: git submodule update --init --recursive")
endif()
if(UMICOM_OS_GUI)
    set(UMICOM_BUILD_GTK4_ADAPTER ON CACHE BOOL "Build Framework GTK4 adapter" FORCE)
endif()
add_subdirectory(framework)
add_subdirectory(applications/os)
