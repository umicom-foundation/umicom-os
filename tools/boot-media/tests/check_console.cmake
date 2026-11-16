# Umicom OS
# File: tools/boot-media/tests/check_console.cmake
#
# PURPOSE:
#   Check harmless installed-console help and argument refusals without inventory, images or device operations.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT

cmake_minimum_required(VERSION 3.25)
if(NOT EXISTS "${EXECUTABLE}" OR NOT DEFINED CASE)
    message(FATAL_ERROR "EXECUTABLE and CASE are required")
endif()
set(_arguments)
set(_expected 2)
set(_needle "")
if(CASE STREQUAL "help")
    set(_arguments --help)
    set(_expected 0)
    set(_needle "Umicom Boot Media")
elseif(CASE STREQUAL "help-word")
    set(_arguments help)
    set(_expected 0)
    set(_needle "Structural inspection and matching bytes do not establish bootability.")
elseif(CASE STREQUAL "self-test")
    set(_arguments --self-test)
    set(_expected 0)
    set(_needle "Native media CRC self-check passed. No file or device was opened.")
elseif(CASE STREQUAL "missing-command")
    # No arguments: the shared parser must report usage, not choose an operation.
elseif(CASE STREQUAL "unknown-command")
    set(_arguments unknown)
elseif(CASE STREQUAL "unicode-command")
    set(_arguments "inconnu-λ")
elseif(CASE STREQUAL "missing-image")
    set(_arguments inspect --image)
elseif(CASE STREQUAL "duplicate-option")
    set(_arguments inspect --image unused --image unused)
elseif(CASE STREQUAL "unexpected-option")
    set(_arguments inspect --unexpected unused)
else()
    message(FATAL_ERROR "Unknown console case")
endif()
# These argument shapes exit before any filesystem or discovery service. Never
# add list/review/copy/write here: physical and media integration are separate.
execute_process(COMMAND "${EXECUTABLE}" ${_arguments}
    RESULT_VARIABLE _result OUTPUT_VARIABLE _stdout ERROR_VARIABLE _stderr TIMEOUT 15)
if(NOT _result STREQUAL "${_expected}")
    message(FATAL_ERROR "Console case ${CASE}: exit ${_result}, expected ${_expected}: ${_stderr}")
endif()
if(_needle)
    string(FIND "${_stdout}" "${_needle}" _found)
    if(_found LESS 0)
        message(FATAL_ERROR "Console case ${CASE}: expected text missing")
    endif()
endif()
if(_stderr)
    message(FATAL_ERROR "Console case ${CASE}: unexpected error output: ${_stderr}")
endif()
