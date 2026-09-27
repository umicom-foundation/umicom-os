# Umicom Foundation | Sammy Hegab | MIT
cmake_minimum_required(VERSION 3.25)
project(UmicomOSBootMedia LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Canonical Framework checkout containing native boot-media services")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/boot_media/CMakeLists.txt")
    message(FATAL_ERROR "Select the canonical Framework checkout with the boot-media tools")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/boot_media" framework-boot-media)
