#-----------------------------------------------------------------------------
# Umicom OS
# File: boot/cvm-probe/CMakeLists.txt
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
project(UmicomOsCvmProbe LANGUAGES C)
include(CTest)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(UMICOM_C_SOURCE_DIR "" CACHE PATH "Umicom C checkout used to build a target-native runtime")
if(NOT EXISTS "${UMICOM_C_SOURCE_DIR}/cmake/UmicomCRuntime.cmake")
    message(FATAL_ERROR "Set UMICOM_C_SOURCE_DIR to the Umicom C source checkout")
endif()
if(NOT EXISTS "${UMICOM_C_SOURCE_DIR}/include/umicom_c/trace.h")
    message(FATAL_ERROR "Select an Umicom C source checkout containing the caller-owned trace API")
endif()
include("${UMICOM_C_SOURCE_DIR}/cmake/UmicomCRuntime.cmake")
umicom_c_add_runtime(umicom_os_cvm_runtime)
add_executable(umicom-cvm-probe main.c)
target_link_libraries(umicom-cvm-probe PRIVATE umicom_os_cvm_runtime)
option(UMICOM_OS_CVM_STATIC "Link the Linux guest probe statically" OFF)
if(UMICOM_OS_CVM_STATIC)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR "The static guest probe requires a Linux target toolchain")
    endif()
    target_link_options(umicom-cvm-probe PRIVATE -static)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(umicom-cvm-probe PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()
# A cross build proves neither launchability nor guest startup. Only a native
# host build registers this process check; QEMU boot acceptance remains separate.
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_test(NAME os.cvm.probe COMMAND umicom-cvm-probe)
    set_tests_properties(os.cvm.probe PROPERTIES TIMEOUT 10
        PASS_REGULAR_EXPRESSION "UMICOM_CVM_PROBE_READY value=42")
endif()
install(TARGETS umicom-cvm-probe RUNTIME DESTINATION usr/libexec)

# Keep native process evidence separate from cross compilation and image boot.
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    # Keep the earlier process list for review; cross builds still register no launches.
    # foreach(case IN ITEMS ordinary-output trace-output invalid-option extra-argument)
    foreach(case IN ITEMS ordinary-output trace-output invalid-option extra-argument comparison-output comparison-extra)
        add_test(NAME "os.cvm.probe.${case}"
            COMMAND "${CMAKE_COMMAND}" "-DTOOL=$<TARGET_FILE:umicom-cvm-probe>" "-DCASE=${case}"
                -P "${CMAKE_CURRENT_LIST_DIR}/check_process.cmake")
        set_tests_properties("os.cvm.probe.${case}" PROPERTIES TIMEOUT 15 LABELS "os;cvm;native;trace")
    endforeach()
endif()
