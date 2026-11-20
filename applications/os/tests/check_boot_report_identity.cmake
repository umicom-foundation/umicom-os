#-----------------------------------------------------------------------------
# Umicom OS
# File: applications/os/tests/check_boot_report_identity.cmake
#
# PURPOSE:
#   Verify exact captured report identity using read-only CLI fixtures.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------
if(NOT DEFINED PROGRAM OR NOT DEFINED CASE OR NOT DEFINED WORK)
    message(FATAL_ERROR "Report identity check requires PROGRAM, CASE and WORK")
endif()
file(MAKE_DIRECTORY "${WORK}")
set(source "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef")
set(capture "${WORK}/captured.report")
if(CASE STREQUAL "unicode")
    set(capture "${WORK}/rapport-café.report")
endif()
set(report "UMICOM_BOOT_REPORT 1\nmode=normal\nstate=ready\nplanned=2\ncompleted=2\nreason=none\nsource=${source}\n")
set(reordered "UMICOM_BOOT_REPORT 1\nsource=${source}\nreason=none\ncompleted=2\nplanned=2\nstate=ready\nmode=normal\n")
string(SHA256 framed "${report}")
file(CONFIGURE OUTPUT "${capture}" CONTENT "${report}" @ONLY NEWLINE_STYLE LF)
set(arguments --boot-report-identity "${capture}")
set(status 0)
set(diagnostic "")

# Keep grammar rejections separate from full source or byte-identity refusal.
if(CASE STREQUAL "match")
    list(APPEND arguments --source "${source}" --capture "${framed}")
elseif(CASE STREQUAL "reverse-flags")
    list(APPEND arguments --capture "${framed}" --source "${source}")
elseif(CASE STREQUAL "report-changed")
    file(CONFIGURE OUTPUT "${capture}" CONTENT "${reordered}" @ONLY NEWLINE_STYLE LF)
    list(APPEND arguments --source "${source}" --capture "${framed}")
    set(status 1)
    set(diagnostic "status=2)")
elseif(CASE STREQUAL "wrong-source")
    string(REPEAT "c" 64 mismatch)
    list(APPEND arguments --source "${mismatch}" --capture "${framed}")
    set(status 1)
    set(diagnostic "status=2)")
elseif(CASE STREQUAL "bad-sha")
    list(APPEND arguments --capture "${framed}0")
    set(status 1)
    set(diagnostic "status=1)")
elseif(CASE STREQUAL "missing-value")
    list(APPEND arguments --capture)
    set(status 2)
elseif(CASE STREQUAL "duplicate")
    list(APPEND arguments --capture "${framed}" --capture "${framed}")
    set(status 2)
elseif(CASE STREQUAL "unknown")
    list(APPEND arguments --different "${framed}")
    set(status 2)
elseif(NOT CASE STREQUAL "plain" AND NOT CASE STREQUAL "unicode")
    message(FATAL_ERROR "Unknown captured report identity case")
endif()

file(SHA256 "${capture}" before)
file(SIZE "${capture}" size)
execute_process(COMMAND "${PROGRAM}" ${arguments} RESULT_VARIABLE actual
    OUTPUT_VARIABLE report_text ERROR_VARIABLE error_text TIMEOUT 10)
file(SHA256 "${capture}" after)
if(NOT before STREQUAL after)
    message(FATAL_ERROR "Report inspection modified the source fixture")
endif()
if(actual STREQUAL "1" AND error_text MATCHES "^The selected Framework lacks capture identity[.]" AND report_text STREQUAL "" AND NOT status EQUAL 2)
    message("Selected Framework has no capture identity API")
    return()
endif()
if(NOT actual STREQUAL "${status}")
    message(FATAL_ERROR "Unexpected result ${actual}; expected ${status}\n${report_text}\n${error_text}")
endif()
if(status EQUAL 0)
    foreach(token IN ITEMS "Captured boot report identity" "source=${source}" "capture=${before}" "bytes=${size}" "state=ready")
        string(FIND "${report_text}" "${token}" found)
        if(found LESS 0 OR NOT error_text STREQUAL "")
            message(FATAL_ERROR "Incomplete capture result: ${report_text}\n${error_text}")
        endif()
    endforeach()
elseif(NOT report_text STREQUAL "")
    message(FATAL_ERROR "A refused request published a result: ${report_text}")
elseif(NOT diagnostic STREQUAL "")
    string(FIND "${error_text}" "${diagnostic}" found)
    if(found LESS 0)
        message(FATAL_ERROR "Refusal did not reach the expected check: ${error_text}")
    endif()
endif()
