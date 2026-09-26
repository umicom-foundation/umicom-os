# Umicom Foundation | Sammy Hegab | MIT
# Host-side release inspection composes the Framework implementation. This
# target does not build, install, boot or write an operating-system image.
cmake_minimum_required(VERSION 3.24)
project(UmicomOSReleaseInspector LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Explicit reviewed Framework source checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/release_inspector/CMakeLists.txt")
    message(FATAL_ERROR "Select a Framework checkout containing the native release inspector")
endif()
set(UMICOM_SETUP_CANONICAL_PROCESS ON CACHE BOOL "Use the real Framework native process runner")
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/release_inspector" framework-release-inspector)
