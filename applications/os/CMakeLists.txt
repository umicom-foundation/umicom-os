#-----------------------------------------------------------------------------
# Umicom OS
# File: applications/os/CMakeLists.txt
#
# PURPOSE:
#   Build the user-space Umicom OS application as a normal Framework consumer.
#
# Created by: Sammy Hegab
# Organisation: Umicom Foundation
# Licence: MIT
#-----------------------------------------------------------------------------
set(UMICOM_OS_CORE_SOURCES
    src/application.c src/desktop.c src/launcher.c src/settings.c
    src/control_centre.c src/session.c src/profile.c src/recovery.c src/services.c
)
add_library(umicom_os_core STATIC ${UMICOM_OS_CORE_SOURCES})
target_include_directories(umicom_os_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(umicom_os_core PUBLIC Umicom::base Umicom::runtime Umicom::platform Umicom::security Umicom::ui Umicom::ui_components)
umicom_apply_warnings(umicom_os_core)
umicom_apply_sanitizers(umicom_os_core)
add_executable(umicom-os src/main.c)
target_link_libraries(umicom-os PRIVATE umicom_os_core)
umicom_apply_warnings(umicom-os)
if(UMICOM_OS_GUI)
    if(NOT TARGET Umicom::ui_gtk4)
        message(FATAL_ERROR "UMICOM_OS_GUI requires the Framework GTK4 adapter")
    endif()
    add_executable(umicom-os-shell src/gtk4/main_gtk4.c)
    target_link_libraries(umicom-os-shell PRIVATE umicom_os_core Umicom::ui_gtk4)
    umicom_apply_warnings(umicom-os-shell)
endif()
if(UMICOM_OS_BUILD_TESTS)
    enable_testing()
    foreach(TEST_NAME application desktop launcher settings session profile recovery services)
        add_executable(umicom-os-test-${TEST_NAME} tests/test_${TEST_NAME}.c)
        target_link_libraries(umicom-os-test-${TEST_NAME} PRIVATE umicom_os_core)
        umicom_apply_warnings(umicom-os-test-${TEST_NAME})
        add_test(NAME os.${TEST_NAME} COMMAND umicom-os-test-${TEST_NAME})
    endforeach()
endif()
