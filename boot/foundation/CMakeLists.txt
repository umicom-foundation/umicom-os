#-----------------------------------------------------------------------------
# Umicom OS | Sammy Hegab, Umicom Foundation | Licence: MIT
# Build directly with -S boot/foundation. The existing root user-space build
# and its Framework submodule are untouched; recovery has no such dependency.
#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
project(UmicomOsFoundation LANGUAGES C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
include(CTest)
option(UMICOM_OS_STATIC "Statically link the guest executables" OFF)
add_library(umicom_os_boot_config STATIC config.c)
target_include_directories(umicom_os_boot_config PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(umicom_os_supervisor STATIC supervisor.c report.c)
    target_include_directories(umicom_os_supervisor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(umicom-os-init init.c console.c)
    target_link_libraries(umicom-os-init PRIVATE umicom_os_boot_config umicom_os_supervisor)
    add_executable(umicom-platform-check platform_check.c)
    if(UMICOM_OS_STATIC)
        target_link_options(umicom-os-init PRIVATE -static)
        target_link_options(umicom-platform-check PRIVATE -static)
    endif()
endif()
if(BUILD_TESTING)
    add_executable(umicom-os-config-tests ../../tests/foundation/test_config.c)
    target_link_libraries(umicom-os-config-tests PRIVATE umicom_os_boot_config)
    foreach(case IN ITEMS valid forward cycle missing duplicate hostname path timeout count options malformed transactional fuzz)
        add_test(NAME "os.foundation.config.${case}" COMMAND umicom-os-config-tests "${case}")
        set_tests_properties("os.foundation.config.${case}" PROPERTIES TIMEOUT 15)
    endforeach()
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_executable(umicom-os-test-child ../../tests/foundation/child.c)
        target_compile_options(umicom-os-test-child PRIVATE -fno-sanitize=all)
        target_link_options(umicom-os-test-child PRIVATE -fno-sanitize=all -static)
        add_executable(umicom-os-supervisor-tests ../../tests/foundation/test_supervisor.c)
        target_link_libraries(umicom-os-supervisor-tests PRIVATE umicom_os_supervisor)
        if(CMAKE_C_FLAGS MATCHES "fsanitize")
            # ASan needs its host runtime and virtual-address layout; a tiny
            # private chroot is not a supported environment for that runtime.
            target_compile_definitions(umicom-os-supervisor-tests PRIVATE UMICOM_TEST_SANITIZED=1)
        endif()
        foreach(case IN ITEMS success failed signal timeout exec format childgroup descriptors limits identity invalid report)
            add_test(NAME "os.foundation.supervisor.${case}"
                COMMAND umicom-os-supervisor-tests "${case}" "$<TARGET_FILE:umicom-os-test-child>")
            set_tests_properties("os.foundation.supervisor.${case}" PROPERTIES TIMEOUT 15 SKIP_RETURN_CODE 77)
        endforeach()
        add_test(NAME os.foundation.pid_guard COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/test_pid_guard.py" "$<TARGET_FILE:umicom-os-init>")
    endif()
endif()

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)
    set(_image_tests "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/test_image.py")
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${_image_tests}" --list
        OUTPUT_VARIABLE _image_cases OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _image_list_status)
    if(NOT _image_list_status EQUAL 0)
        message(FATAL_ERROR "Could not discover the image format tests")
    endif()
    string(REPLACE "\n" ";" _image_cases "${_image_cases}")
    foreach(_case IN LISTS _image_cases)
        add_test(NAME "os.foundation.image.${_case}" COMMAND "${Python3_EXECUTABLE}"
            "${_image_tests}" "ImageTests.${_case}")
        set_tests_properties("os.foundation.image.${_case}" PROPERTIES TIMEOUT 20)
    endforeach()
endif()

# Cross-repository protocol check is optional and belongs to the host suite,
# not the independent recovery binary. Pass the current Framework source root.
set(UMICOM_OS_FRAMEWORK_SOURCE "" CACHE PATH "Framework source for the host boot-report contract check")
if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND UMICOM_OS_FRAMEWORK_SOURCE)
    add_executable(umicom-os-report-contract
        ../../tests/foundation/test_report_contract.c
        "${UMICOM_OS_FRAMEWORK_SOURCE}/src/platform/boot_report.c")
    target_include_directories(umicom-os-report-contract PRIVATE "${UMICOM_OS_FRAMEWORK_SOURCE}/include")
    target_link_libraries(umicom-os-report-contract PRIVATE umicom_os_supervisor)
    add_test(NAME os.foundation.report_contract COMMAND umicom-os-report-contract)
    set_tests_properties(os.foundation.report_contract PROPERTIES TIMEOUT 15)
endif()

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    add_executable(umicom-os-console-driver ../../tests/foundation/console_driver.c console.c)
    target_include_directories(umicom-os-console-driver PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach(_case IN ITEMS help unknown path long control escape reboot blank)
        add_test(NAME "os.foundation.console.${_case}" COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/test_console.py"
            "$<TARGET_FILE:umicom-os-console-driver>" "${_case}")
        set_tests_properties("os.foundation.console.${_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()
