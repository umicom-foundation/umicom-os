#-----------------------------------------------------------------------------
# Umicom OS
# File: tools/cvm-preview/CMakeLists.txt
#
# PURPOSE:
#   Reuse Umicom C through a bounded, optional user-space consumer.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#---------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomOsCvmPreview LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework"
    CACHE PATH "Framework source containing the shared CVM preview service")
get_filename_component(_framework "${UMICOM_OS_FRAMEWORK_SOURCE}" ABSOLUTE)
if(NOT EXISTS "${_framework}/examples/cvm_preview/CMakeLists.txt")
    message(FATAL_ERROR "Select the published Framework checkout containing examples/cvm_preview")
endif()
# Reuse the service and its tests directly. The full desktop and boot recovery
# dependency graphs are not pulled into this optional command-line consumer.
add_subdirectory("${_framework}/examples/cvm_preview"
    "${CMAKE_CURRENT_BINARY_DIR}/framework-preview")
