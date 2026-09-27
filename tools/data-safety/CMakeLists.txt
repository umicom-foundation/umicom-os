# Umicom Foundation | Sammy Hegab | MIT
# Framework owns the implementation. This OS-host entry composes its checks;
# it does not create another Data Server or modify a guest image.
cmake_minimum_required(VERSION 3.24)
project(UmicomOsDataSafety LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Existing canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/data_safety/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing the reconstructed transaction-safety update")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/data_safety" framework-data-safety)
