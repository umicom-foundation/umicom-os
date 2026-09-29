# Umicom Foundation | MIT. Thin host: all implementation and tests are Framework-owned.
cmake_minimum_required(VERSION 3.24)
project(UmicomThreadSafetyHost LANGUAGES C)
include(CTest)
set(UMICOM_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework"
    CACHE PATH "Framework source checkout")
get_filename_component(UMICOM_FRAMEWORK_SOURCE "${UMICOM_FRAMEWORK_SOURCE}" ABSOLUTE)
if(NOT EXISTS "${UMICOM_FRAMEWORK_SOURCE}/examples/thread_safety/CMakeLists.txt")
    message(FATAL_ERROR "Select the reviewed Framework checkout with -DUMICOM_FRAMEWORK_SOURCE=...")
endif()
add_subdirectory("${UMICOM_FRAMEWORK_SOURCE}/examples/thread_safety"
    "${CMAKE_CURRENT_BINARY_DIR}/framework-thread-safety")
