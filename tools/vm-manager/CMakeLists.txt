# Umicom Foundation | Sammy Hegab | MIT
cmake_minimum_required(VERSION 3.25)
project(UmicomOSVmManager LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/vm_manager/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing the VM manager")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/vm_manager" framework-vm-manager)
