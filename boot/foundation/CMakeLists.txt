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
add_library(umicom_os_boot_config STATIC config.c admission.c)
add_library(umicom_os_guest_environment STATIC guest_environment.c)
target_include_directories(umicom_os_guest_environment PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(umicom_os_boot_config PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# Share the production parser with an unprivileged inspection tool. The tool
# is a host review surface and is not added to the guest rootfs inventory.
add_executable(umicom-os-boot-plan plan.c)
target_link_libraries(umicom-os-boot-plan PRIVATE umicom_os_boot_config)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(umicom-os-boot-plan PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(umicom_os_supervisor STATIC supervisor.c report.c)
    target_include_directories(umicom_os_supervisor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(umicom-os-init init.c console.c)
    target_link_libraries(umicom-os-init PRIVATE umicom_os_boot_config umicom_os_supervisor)
    add_executable(umicom-platform-check platform_check.c)
    target_link_libraries(umicom-platform-check PRIVATE umicom_os_guest_environment)
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
    target_link_libraries(umicom-os-report-contract PRIVATE umicom_os_supervisor umicom_os_boot_config)
    add_test(NAME os.foundation.report_contract COMMAND umicom-os-report-contract)
    set_tests_properties(os.foundation.report_contract PROPERTIES TIMEOUT 15)
endif()

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    add_executable(umicom-os-console-driver ../../tests/foundation/console_driver.c console.c)
    target_link_libraries(umicom-os-console-driver PRIVATE umicom_os_boot_config)
    target_include_directories(umicom-os-console-driver PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach(_case IN ITEMS help unknown path long control escape reboot blank)
        add_test(NAME "os.foundation.console.${_case}" COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/test_console.py"
            "$<TARGET_FILE:umicom-os-console-driver>" "${_case}")
        set_tests_properties("os.foundation.console.${_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

# The inspector uses the production parser but never starts services.
if(BUILD_TESTING)
    foreach(_plan_case IN ITEMS ordered recovery cycle unsafe-path duplicate-option missing unknown-argument)
        add_test(NAME "os.foundation.plan.${_plan_case}"
            COMMAND "${CMAKE_COMMAND}"
                "-DPROGRAM=$<TARGET_FILE:umicom-os-boot-plan>"
                "-DPLAN_CASE=${_plan_case}"
                "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/plans"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/check_plan.cmake")
        set_tests_properties("os.foundation.plan.${_plan_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(umicom-os-config-diagnostic-tests ../../tests/foundation/test_config_diagnostic.c)
    target_link_libraries(umicom-os-config-diagnostic-tests PRIVATE umicom_os_boot_config)
    foreach(_diagnostic_case IN ITEMS valid argument format character hostname service capacity field required graph)
        add_test(NAME "os.foundation.diagnostic.${_diagnostic_case}" COMMAND umicom-os-config-diagnostic-tests "${_diagnostic_case}")
        set_tests_properties("os.foundation.diagnostic.${_diagnostic_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(umicom-os-admission-tests ../../tests/foundation/test_admission.c)
    target_link_libraries(umicom-os-admission-tests PRIVATE umicom_os_boot_config)
    foreach(_admission_case IN ITEMS normal recovery hostname-recovery mount untrusted-options empty bounds arguments matrix)
        add_test(NAME "os.foundation.admission.${_admission_case}" COMMAND umicom-os-admission-tests "${_admission_case}")
        set_tests_properties("os.foundation.admission.${_admission_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

# Check the independent foundation at the same warning level as its guest Makefile.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    foreach(_foundation_target IN ITEMS umicom_os_boot_config umicom_os_supervisor
        umicom-os-init umicom-platform-check umicom-os-config-tests
        umicom-os-config-diagnostic-tests umicom-os-admission-tests)
        if(TARGET ${_foundation_target})
            target_compile_options(${_foundation_target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        endif()
    endforeach()
endif()

if(BUILD_TESTING AND TARGET umicom-os-console-driver)
    foreach(_console_plan_case IN ITEMS services empty)
        add_test(NAME "os.foundation.console.${_console_plan_case}"
            COMMAND "${CMAKE_COMMAND}"
                "-DPROGRAM=$<TARGET_FILE:umicom-os-console-driver>"
                "-DCASE=${_console_plan_case}"
                "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/console-${_console_plan_case}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/check_console_plan.cmake")
        set_tests_properties("os.foundation.console.${_console_plan_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

# Pure observation-policy tests run on the host without guest syscalls.
if(BUILD_TESTING)
    add_executable(umicom-os-guest-environment-tests ../../tests/foundation/test_guest_environment.c)
    target_link_libraries(umicom-os-guest-environment-tests PRIVATE umicom_os_guest_environment)
    foreach(_guest_case IN ITEMS valid argument missing-observation identity groups privileges
        core-limit file-limit descriptor-limit hard-limits workspace-owner workspace-mode
        workspace-location mounts unchanged messages)
        add_test(NAME "os.foundation.guest.${_guest_case}" COMMAND umicom-os-guest-environment-tests "${_guest_case}")
        set_tests_properties("os.foundation.guest.${_guest_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(umicom_os_guest_environment PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(TARGET umicom-os-guest-environment-tests)
        target_compile_options(umicom-os-guest-environment-tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
endif()

# Admission fixtures use real child processes only on the Linux host.
if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_CROSSCOMPILING)
    add_executable(umicom-os-supervisor-admission-tests
        ../../tests/foundation/test_supervisor_admission.c)
    target_link_libraries(umicom-os-supervisor-admission-tests PRIVATE umicom_os_supervisor)
    target_compile_options(umicom-os-supervisor-admission-tests PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    foreach(_owner_case IN ITEMS ignored-child auto-reap readonly-log closed-log
        closed-streams closed-streams-exec low-log caller-log)
        add_test(NAME "os.foundation.supervisor_owner.${_owner_case}"
            COMMAND umicom-os-supervisor-admission-tests "${_owner_case}"
                "$<TARGET_FILE:umicom-os-test-child>")
        set_tests_properties("os.foundation.supervisor_owner.${_owner_case}" PROPERTIES TIMEOUT 15)
    endforeach()
endif()

# Structured inspection uses the same unprivileged parser executable. These
# checks do not invoke PID1, mount filesystems or start a configured service.
if(BUILD_TESTING)
    foreach(_json_case IN ITEMS normal recovery option-order poweroff cycle unsafe-path
            duplicate-json duplicate-cmdline missing-value option-as-value unknown invalid-controls)
        add_test(NAME "os.foundation.plan.json.${_json_case}"
            COMMAND "${CMAKE_COMMAND}"
                "-DPROGRAM=$<TARGET_FILE:umicom-os-boot-plan>"
                "-DPLAN_CASE=${_json_case}"
                "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/plans"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../../tests/foundation/check_plan_json.cmake")
        set_tests_properties("os.foundation.plan.json.${_json_case}" PROPERTIES
            TIMEOUT 15 LABELS "os;foundation;inspection;regression")
    endforeach()
endif()
