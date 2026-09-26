# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Thin composition; installation and VM/provider logic stay in Framework.
cmake_minimum_required(VERSION 3.24)
project(UmicomOSSetupCentre LANGUAGES C)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
    "Explicit reviewed Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/setup_centre/CMakeLists.txt")
    message(FATAL_ERROR "Select a Framework checkout containing Setup Centre")
endif()
include(CTest)
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/setup_centre" "setup-centre")
