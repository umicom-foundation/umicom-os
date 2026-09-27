# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
cmake_minimum_required(VERSION 3.24)
project(UmicomOSDesktopWorkspace LANGUAGES C)
include(CTest)
get_filename_component(_default_framework "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
set(UMICOM_OS_FRAMEWORK_SOURCE "${_default_framework}" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/desktop_workspace/CMakeLists.txt")
    message(FATAL_ERROR "Set UMICOM_OS_FRAMEWORK_SOURCE to the updated Umicom Framework checkout")
endif()
# This is a host-tool composition, not an OS-image builder or GTK guest.
# Kernel and recovery stay independent; no duplicate storage implementation.
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/desktop_workspace"
    "${CMAKE_CURRENT_BINARY_DIR}/framework-workspace")
