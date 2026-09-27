# Umicom Foundation | Sammy Hegab | MIT
# A host-side composition of the shared Framework gallery and native checks.
# This does not build a compositor, duplicate layout state or modify a guest.
cmake_minimum_required(VERSION 3.24)
project(UmicomOsComponentWorkbench LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Existing canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/workbench_viewport/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing the shared workbench viewport")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/workbench_viewport" framework-workbench)
