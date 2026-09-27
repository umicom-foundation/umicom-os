# Umicom Foundation | Sammy Hegab | MIT
cmake_minimum_required(VERSION 3.24)
project(UmicomResearchHost LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/research_replay/CMakeLists.txt")
    message(FATAL_ERROR "Select the canonical Framework with research replay sources")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/research_replay" research-replay)
