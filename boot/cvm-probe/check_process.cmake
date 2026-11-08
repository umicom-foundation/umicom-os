#-----------------------------------------------------------------------------
# Umicom OS
# File: boot/cvm-probe/check_process.cmake
#
# PURPOSE:
#   Check complete native probe transcripts and reject unsupported arguments.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(NOT DEFINED TOOL OR NOT DEFINED CASE)
    message(FATAL_ERROR "TOOL and CASE are required")
endif()
set(args)
set(expected_exit 0)
set(expected "UMICOM_CVM_PROBE_READY value=42\n")
set(expected_error "")
if(CASE STREQUAL "ordinary-output")
elseif(CASE STREQUAL "trace-output")
    list(APPEND args --trace-check)
    set(expected "UMICOM_CVM_TRACE_READY cases=4\n")
elseif(CASE STREQUAL "comparison-output")
    list(APPEND args --comparison-check)
    set(expected "UMICOM_CVM_COMPARISON_READY cases=8\n")
# Earlier invalid-option condition retained; reject extra comparison arguments too.
# elseif(CASE STREQUAL "invalid-option" OR CASE STREQUAL "extra-argument")
elseif(CASE STREQUAL "invalid-option" OR CASE STREQUAL "extra-argument" OR CASE STREQUAL "comparison-extra")
    if(CASE STREQUAL "invalid-option")
        list(APPEND args --unknown)
    elseif(CASE STREQUAL "comparison-extra")
        list(APPEND args --comparison-check unexpected)
    else()
        list(APPEND args --trace-check unexpected)
    endif()
    set(expected_exit 2)
    set(expected "")
    # Previous usage retained for review.
    # set(expected_error "Usage: umicom-cvm-probe [--trace-check]\n")
    set(expected_error "Usage: umicom-cvm-probe [--trace-check|--comparison-check]\n")
else()
    message(FATAL_ERROR "Unknown native probe case")
endif()
execute_process(COMMAND "${TOOL}" ${args} RESULT_VARIABLE result
    OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 10)
string(REPLACE "\r\n" "\n" output "${output}")
string(REPLACE "\r\n" "\n" error "${error}")
if(NOT "${result}" STREQUAL "${expected_exit}" OR NOT output STREQUAL expected OR
        NOT error STREQUAL expected_error)
    message(FATAL_ERROR "Unexpected native probe outcome: ${result}: ${output}: ${error}")
endif()
