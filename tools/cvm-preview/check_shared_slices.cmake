# Umicom OS
# File: tools/cvm-preview/check_shared_slices.cmake
#
# PURPOSE:
#   Verify the OS wrapper consumes the Framework slice command without another interpreter.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
if(NOT DEFINED TOOL OR NOT EXISTS "${TOOL}")
    message(FATAL_ERROR "Select the built shared preview executable")
endif()
execute_process(COMMAND "${TOOL}" --slice 2 "int main(void){return 6*7;}"
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 5)
string(STRIP "${out}" out)
if(NOT "${status}" STREQUAL "0" OR NOT "${out}" STREQUAL "slices=2 instructions=4 value=42")
    message(FATAL_ERROR "Shared slice consumer failed: ${status} / ${out} / ${err}")
endif()
execute_process(COMMAND "${TOOL}" --slice 0
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 5)
if(NOT "${status}" STREQUAL "2" OR NOT "${out}" STREQUAL "")
    message(FATAL_ERROR "Zero scheduling allowance must be refused")
endif()
