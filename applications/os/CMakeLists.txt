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
    # The shell composes the shared tools through a reusable target, allowing
    # native lifetime checks to exercise exactly the same implementation.
    foreach(_tool IN ITEMS DesktopWorkspace DesktopSystem)
        include("${PROJECT_SOURCE_DIR}/framework/cmake/Umicom${_tool}Gtk4.cmake")
    endforeach()
    add_library(umicom_os_desktop_shell STATIC src/gtk4/desktop_shell.c)
    target_link_libraries(umicom_os_desktop_shell PUBLIC umicom_os_core
        Umicom::desktop_workspace_gtk4 Umicom::desktop_system_gtk4)
    umicom_apply_warnings(umicom_os_desktop_shell)
    umicom_apply_sanitizers(umicom_os_desktop_shell)
    add_executable(umicom-os-shell src/gtk4/main_gtk4.c)
    target_link_libraries(umicom-os-shell PRIVATE umicom_os_desktop_shell)
    if(UMICOM_OS_BUILD_TESTS)
        add_executable(umicom-os-desktop-shell-test tests/test_desktop_shell.c)
        target_link_libraries(umicom-os-desktop-shell-test PRIVATE umicom_os_desktop_shell)
        umicom_apply_warnings(umicom-os-desktop-shell-test)
        umicom_apply_sanitizers(umicom-os-desktop-shell-test)
        add_test(NAME os.desktop_shell.native COMMAND umicom-os-desktop-shell-test)
        set_tests_properties(os.desktop_shell.native PROPERTIES SKIP_RETURN_CODE 77
            TIMEOUT 30 LABELS "os;desktop;gtk4;ownership")
    endif()
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

# The parent installs Framework separately. Register the OS programs as their
# own component so an installation includes the product a user can launch.
# Future OS executables should join this component rather than relying on a
# Framework SDK installation to discover application-owned targets.
include(GNUInstallDirs)
install(TARGETS umicom-os
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT UmicomOS)
if(TARGET umicom-os-shell)
    install(TARGETS umicom-os-shell
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT UmicomOS)
endif()

if(UMICOM_OS_BUILD_TESTS)
    # Qualify installation in a build-owned staging directory. The fixture
    # verifies byte identity without launching a desktop during CTest.
    set(_umicom_os_install_shell "")
    if(TARGET umicom-os-shell)
        set(_umicom_os_install_shell "$<TARGET_FILE:umicom-os-shell>")
    endif()
    add_test(NAME os.install.programs
        COMMAND "${CMAKE_COMMAND}"
            "-DUMICOM_INSTALL_BINARY_DIR=${CMAKE_BINARY_DIR}"
            "-DUMICOM_INSTALL_TEST_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/install-check/$<CONFIG>"
            "-DUMICOM_INSTALL_CONFIG=$<CONFIG>"
            "-DUMICOM_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}"
            "-DUMICOM_INSTALL_CONSOLE=$<TARGET_FILE:umicom-os>"
            "-DUMICOM_INSTALL_SHELL=${_umicom_os_install_shell}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_install.cmake")
    set_tests_properties(os.install.programs PROPERTIES
        TIMEOUT 60 LABELS "os;installation;regression")
    unset(_umicom_os_install_shell)
endif()

if(UMICOM_OS_BUILD_TESTS AND BUILD_TESTING)
    foreach(_report_case IN ITEMS valid matching mismatch malformed relative unknown help unicode)
        add_test(NAME "os.boot_report_cli.${_report_case}"
            COMMAND "${CMAKE_COMMAND}"
                "-DPROGRAM=$<TARGET_FILE:umicom-os>"
                "-DCASE=${_report_case}"
                "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/report-cli/${_report_case}/$<CONFIG>"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_boot_report.cmake")
        set_tests_properties("os.boot_report_cli.${_report_case}" PROPERTIES TIMEOUT 15 LABELS "os;boot-report;cli")
    endforeach()
endif()
