#-----------------------------------------------------------------------------
# Umicom OS
# File: tools/cvm-preview/check_shared_artifact.cmake
#
# PURPOSE:
#   Exercise exact native bytecode and truncated-input refusal through Framework.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#-----------------------------------------------------------------------------

string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef fixture)
set(work "${WORK}/fixture-${fixture}")
file(MAKE_DIRECTORY "${work}")
set(source "${work}/answer.uc")
set(artifact "${work}/compiled answer.umcb")
set(truncated "${work}/truncated answer.umcb")
file(WRITE "${source}" "int main(void) { return (20 + 1) * 2; }\n")

# The compiler and consumer sit on opposite sides of the shared wire contract.
execute_process(COMMAND "${COMPILER}" compile "${source}" "${artifact}"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 8)
if(NOT result STREQUAL "0" OR NOT EXISTS "${artifact}")
    message(FATAL_ERROR "Cannot compile the independent bytecode fixture. ${result} ${out} ${err}")
endif()
file(SHA256 "${artifact}" before)
execute_process(COMMAND "${TOOL}" --bytecode "${artifact}" RESULT_VARIABLE result
    OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 8 OUTPUT_STRIP_TRAILING_WHITESPACE)
file(SHA256 "${artifact}" after)
if(NOT result STREQUAL "0" OR NOT out STREQUAL "42" OR NOT err STREQUAL "" OR NOT before STREQUAL after)
    message(FATAL_ERROR "The shared preview did not return 42 from an unchanged artifact. ${result} ${out} ${err}")
endif()

file(WRITE "${truncated}" "UMCB")
execute_process(COMMAND "${TOOL}" --bytecode "${truncated}" RESULT_VARIABLE result
    OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 8 OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT result STREQUAL "1" OR NOT out STREQUAL "" OR NOT err MATCHES "Bytecode preview:")
    message(FATAL_ERROR "The shared preview did not refuse a truncated artifact. ${result} ${out} ${err}")
endif()
