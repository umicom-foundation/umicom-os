# Umicom Foundation | Sammy Hegab | MIT
# Host application-file maintenance; no OS image, partition or user data edits.
cmake_minimum_required(VERSION 3.24)
project(UmicomOSSetupMaintenance LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Explicit reviewed Framework source checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/setup_maintenance/CMakeLists.txt")
    message(FATAL_ERROR "Select Framework containing the native maintenance implementation")
endif()
set(UMICOM_SETUP_CANONICAL_PROCESS ON CACHE BOOL "Use the canonical native process runner")
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/setup_maintenance" framework-maintenance)
