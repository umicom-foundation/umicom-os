#-----------------------------------------------------------------------------
# Umicom OS
# File: tools/cvm-preview/check_shared_steps.cmake
#
# PURPOSE:
#   Verify OS composition forwards source stepping and runtime faults through Framework.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#---------------------------------------------------------------------------


execute_process(COMMAND "${TOOL}" --steps "int main(void){
int x=40;
x+=2;
return x;
}"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 10)
if(NOT result EQUAL 0 OR NOT out MATCHES "line=3" OR NOT out MATCHES "42[
]+$")
    message(FATAL_ERROR "Shared stepped result missing: ${result} ${out} ${err}")
endif()
execute_process(COMMAND "${TOOL}" --steps "int main(void){
return 1/0;
}"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 10)
if(NOT result EQUAL 1 OR NOT err MATCHES "2:[0-9]+:" OR out MATCHES "42[
]+$")
    message(FATAL_ERROR "Shared fault lost its source location: ${result} ${out} ${err}")
endif()
execute_process(COMMAND "${TOOL}" --steps "int main(void){return missing;}"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 10)
if(NOT result EQUAL 1 OR out MATCHES "step=")
    message(FATAL_ERROR "Invalid source started execution")
endif()
