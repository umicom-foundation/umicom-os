# Umicom Foundation | Sammy Hegab | MIT
cmake_minimum_required(VERSION 3.24)
project(UmicomOSBrokerConnect LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/ibkr_connection/CMakeLists.txt")
    message(FATAL_ERROR "Select the updated canonical Framework checkout")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/ibkr_connection" framework)
