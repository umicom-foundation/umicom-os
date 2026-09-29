# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Host tooling only. No kernel, root filesystem or guest service is changed.
cmake_minimum_required(VERSION 3.24)
project(UmicomAiEvidenceHost LANGUAGES C)
include(CTest)
get_filename_component(_framework "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
set(UMICOM_OS_FRAMEWORK_SOURCE "${_framework}" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/ai_evidence/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing examples/ai_evidence")
endif()
set(UMICOM_AI_EVIDENCE_FRAMEWORK_SOURCE "${UMICOM_OS_FRAMEWORK_SOURCE}" CACHE PATH "Actual Framework source" FORCE)
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/ai_evidence" "ai-evidence-host")
