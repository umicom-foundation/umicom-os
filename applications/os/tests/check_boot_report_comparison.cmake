#-----------------------------------------------------------------------------
# Umicom OS
# File: applications/os/tests/check_boot_report_comparison.cmake
#
# PURPOSE:
#   Verify report comparison formatting, rejection and read-only input ownership.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

if(NOT DEFINED PROGRAM OR NOT DEFINED CASE OR NOT DEFINED WORK)
    message(FATAL_ERROR "Report comparison needs PROGRAM, CASE and WORK")
endif()
file(MAKE_DIRECTORY "${WORK}")
set(_before "${WORK}/earlier.report")
set(_after "${WORK}/later.report")
if(CASE STREQUAL "unicode")
    set(_before "${WORK}/rapport-été.report")
    set(_after "${WORK}/rapport-café.report")
endif()
set(_source "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef")
set(_earlier "UMICOM_BOOT_REPORT 1\nmode=normal\nstate=starting\nplanned=2\ncompleted=1\nreason=none\nsource=${_source}\n")
set(_later "${_earlier}")
set(_expected 0)
set(_text "No semantic fields changed.")
if(CASE STREQUAL "equal" OR CASE STREQUAL "unicode")
elseif(CASE STREQUAL "progress")
    string(REPLACE "state=starting" "state=ready" _later "${_later}")
    string(REPLACE "completed=1" "completed=2" _later "${_later}")
    set(_text "state: starting -> ready" "completed: 1 -> 2")
elseif(CASE STREQUAL "source")
    string(REPLACE "source=0" "source=1" _later "${_later}")
    set(_text "Source identities differ; review the boot plans")
elseif(CASE STREQUAL "invalid-before")
    set(_earlier "invalid report\n")
    set(_expected 1)
    set(_text "Cannot read earlier report (status=1).")
elseif(CASE STREQUAL "invalid-after")
    set(_later "invalid report\n")
    set(_expected 1)
    set(_text "Cannot read later report (status=1).")
elseif(CASE STREQUAL "missing-argument")
    set(_expected 2)
    set(_text "--compare-boot-reports EARLIER_ABSOLUTE_FILE LATER_ABSOLUTE_FILE")
else()
    message(FATAL_ERROR "Unknown comparison case")
endif()
# Explicit LF bytes preserve the boot protocol on a Windows host.
file(CONFIGURE OUTPUT "${_before}" CONTENT "${_earlier}" @ONLY NEWLINE_STYLE LF)
file(CONFIGURE OUTPUT "${_after}" CONTENT "${_later}" @ONLY NEWLINE_STYLE LF)
file(SHA256 "${_before}" _before_hash)
file(SHA256 "${_after}" _after_hash)
if(CASE STREQUAL "missing-argument")
    execute_process(COMMAND "${PROGRAM}" --compare-boot-reports "${_before}"
        RESULT_VARIABLE _result OUTPUT_VARIABLE _out ERROR_VARIABLE _err TIMEOUT 10)
else()
    execute_process(COMMAND "${PROGRAM}" --compare-boot-reports "${_before}" "${_after}"
        RESULT_VARIABLE _result OUTPUT_VARIABLE _out ERROR_VARIABLE _err TIMEOUT 10)
endif()
if(NOT "${_result}" STREQUAL "${_expected}")
    message(FATAL_ERROR "Unexpected exit ${_result}; expected ${_expected}\n${_out}\n${_err}")
endif()
foreach(_needle IN LISTS _text)
    string(FIND "${_out}${_err}" "${_needle}" _found)
    if(_found LESS 0)
        message(FATAL_ERROR "Missing ${_needle}\n${_out}\n${_err}")
    endif()
endforeach()
if(_expected EQUAL 0)
    string(FIND "${_out}" "Reports are observations" _found)
    if(_found LESS 0)
        message(FATAL_ERROR "Missing evidence limitations")
    endif()
else()
    string(FIND "${_out}" "Captured boot report comparison" _invented)
    if(NOT _invented LESS 0)
        message(FATAL_ERROR "Refused reports must not produce a successful comparison")
    endif()
endif()
file(SHA256 "${_before}" _before_now)
file(SHA256 "${_after}" _after_now)
if(NOT _before_hash STREQUAL _before_now OR NOT _after_hash STREQUAL _after_now)
    message(FATAL_ERROR "Comparison changed its input files")
endif()
