# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Hosted desktop-session tools. Independent init/recovery and boot profiles
# are not replaced or linked to GTK by this user-space composition.
cmake_minimum_required(VERSION 3.24)
project(UmicomOSDesktopSession LANGUAGES C)
include(CTest)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../framework" CACHE PATH
    "Reviewed Framework source checkout (normally the OS root submodule)")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/desktop_system/CMakeLists.txt")
    message(FATAL_ERROR "Select a Framework checkout containing Desktop System with -DUMICOM_OS_FRAMEWORK_SOURCE=...")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/desktop_system" framework-desktop-system)
