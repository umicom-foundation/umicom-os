# Umicom Foundation | Sammy Hegab | MIT
# This is a host-tool entry, not a guest/kernel build or another trading engine.
cmake_minimum_required(VERSION 3.24)
project(UmicomOsMarketTapeHost LANGUAGES C)
include(CTest)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/market_tape/CMakeLists.txt")
    message(FATAL_ERROR "Select the reviewed Framework source with UMICOM_OS_FRAMEWORK_SOURCE")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/market_tape" "${CMAKE_CURRENT_BINARY_DIR}/framework-market-tape")
