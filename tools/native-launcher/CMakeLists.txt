# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# The primary tool is Framework's C23 executable. Keep desktop_session.py as
# the unchanged alternative; there is no second OS-local delivery engine.
cmake_minimum_required(VERSION 3.24)
project(UmicomOSNativeLauncher LANGUAGES C)
include(CTest)
get_filename_component(_default_framework "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
set(UMICOM_OS_FRAMEWORK_SOURCE "${_default_framework}" CACHE PATH "Umicom Framework source checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/native_launcher/CMakeLists.txt")
    message(FATAL_ERROR "Select a Framework checkout containing the native launcher delivery capability")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/native_launcher" framework-native-launcher)
