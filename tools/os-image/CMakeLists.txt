# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Thin native host-tool composition. Guest boot policy remains in boot/foundation.
cmake_minimum_required(VERSION 3.25)
project(UmicomNativeImageTools LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Canonical Umicom Framework checkout")
get_filename_component(_framework "${UMICOM_OS_FRAMEWORK_SOURCE}" ABSOLUTE)
if(NOT EXISTS "${_framework}/examples/os_image/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing native OS image tools")
endif()
add_subdirectory("${_framework}/examples/os_image" "${CMAKE_CURRENT_BINARY_DIR}/framework-image")
