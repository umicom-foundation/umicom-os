# Umicom Foundation | Sammy Hegab | MIT
cmake_minimum_required(VERSION 3.24)
project(UmicomComponentIntegrationHost LANGUAGES C)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework"
    CACHE PATH "Canonical Framework checkout")
get_filename_component(FRAMEWORK "${UMICOM_OS_FRAMEWORK_SOURCE}" ABSOLUTE)
set(PROBE_REGISTER_MATRIX ON CACHE BOOL "Register integration matrix checks")
include(CTest)
add_subdirectory("${FRAMEWORK}/tests/component_integration/probe" "${CMAKE_CURRENT_BINARY_DIR}/probe")
