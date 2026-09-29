# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Host tools only. This does not build a kernel, boot a guest or mount a volume.
cmake_minimum_required(VERSION 3.24)
project(UmicomOsAudioTools LANGUAGES C)
include(CTest)
get_filename_component(_framework "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
set(UMICOM_OS_FRAMEWORK_SOURCE "${_framework}" CACHE PATH "Complete Framework checkout")
set(UMICOM_FRAMEWORK_SOURCE "${UMICOM_OS_FRAMEWORK_SOURCE}" CACHE PATH "Canonical Framework" FORCE)
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/creative_audio" "${CMAKE_CURRENT_BINARY_DIR}/audio-sdk")
