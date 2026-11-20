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
# The old single-project check is retained; require the shared debugger API as
# well so a stale dependency copy fails during configuration instead of linking.
if(FALSE)
if(NOT EXISTS "${_framework}/examples/cvm_preview/CMakeLists.txt")
    message(FATAL_ERROR "Select the published Framework checkout containing examples/cvm_preview")
endif()
endif()
if(NOT EXISTS "${_framework}/examples/cvm_preview/CMakeLists.txt" OR
   NOT EXISTS "${_framework}/include/umicom/toolchain/cvm_debug.h")
    message(FATAL_ERROR "Select a Framework checkout containing the CVM debugger service; update the dependency or set UMICOM_OS_FRAMEWORK_SOURCE explicitly.")
endif()
# Reuse the service and its tests directly. The full desktop and boot recovery
# dependency graphs are not pulled into this optional command-line consumer.
add_subdirectory("${_framework}/examples/cvm_preview"
    "${CMAKE_CURRENT_BINARY_DIR}/framework-preview")

if(BUILD_TESTING)
    add_test(NAME os.cvm.shared-step-consumer COMMAND "${CMAKE_COMMAND}"
        "-DTOOL=$<TARGET_FILE:umicom-cvm-preview>"
        -P "${CMAKE_CURRENT_LIST_DIR}/check_shared_steps.cmake")
    set_tests_properties(os.cvm.shared-step-consumer PROPERTIES TIMEOUT 20 LABELS "os;cvm;sdk;debugger")
endif()

# No product-local executor: require and exercise the shared slice service.
file(STRINGS "${_framework}/include/umicom/toolchain/cvm_debug.h"
    _slice_api REGEX "UmcStatus umi_cvm_debug_resume")
if(NOT _slice_api)
    message(FATAL_ERROR "Update the selected Framework source to the bounded CVM resume API.")
endif()
if(BUILD_TESTING)
    add_test(NAME os.cvm.shared-slice-consumer COMMAND "${CMAKE_COMMAND}"
        "-DTOOL=$<TARGET_FILE:umicom-cvm-preview>"
        -P "${CMAKE_CURRENT_LIST_DIR}/check_shared_slices.cmake")
    set_tests_properties(os.cvm.shared-slice-consumer PROPERTIES TIMEOUT 15 LABELS "os;cvm;sdk;slices")
endif()

# A selected hosted compiler creates the wire fixture at test time. Match it
# to the SDK used by the Framework consumer and leave source checks available
# when only an older preview-capable dependency is selected.
set(UMICOM_OS_CVM_ARTIFACT_COMPILER "" CACHE FILEPATH
    "Absolute path to the matching hosted umicom-c tool for the artifact contract")
if(BUILD_TESTING AND UMICOM_OS_CVM_ARTIFACT_COMPILER)
    if(NOT IS_ABSOLUTE "${UMICOM_OS_CVM_ARTIFACT_COMPILER}" OR
       NOT EXISTS "${UMICOM_OS_CVM_ARTIFACT_COMPILER}" OR
       IS_DIRECTORY "${UMICOM_OS_CVM_ARTIFACT_COMPILER}")
        message(FATAL_ERROR "Select an existing absolute hosted umicom-c executable for artifact checks.")
    endif()
    file(STRINGS "${_framework}/include/umicom/toolchain/cvm_preview.h"
        _artifact_api REGEX "UmiStatus umi_cvm_preview_bytecode")
    if(NOT _artifact_api)
        message(FATAL_ERROR "The selected Framework preview lacks the shared bytecode API.")
    endif()
    add_test(NAME os.cvm.shared-artifact-consumer COMMAND "${CMAKE_COMMAND}"
        "-DCOMPILER=${UMICOM_OS_CVM_ARTIFACT_COMPILER}"
        "-DTOOL=$<TARGET_FILE:umicom-cvm-preview>"
        "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/artifact-contract"
        -P "${CMAKE_CURRENT_LIST_DIR}/check_shared_artifact.cmake")
    set_tests_properties(os.cvm.shared-artifact-consumer PROPERTIES TIMEOUT 30 LABELS "os;cvm;sdk;bytecode")
endif()
