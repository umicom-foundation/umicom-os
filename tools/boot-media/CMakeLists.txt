# Umicom OS
# File: tools/boot-media/CMakeLists.txt
#
# PURPOSE:
#   Compose the existing boot-media source tools or a thin console using an explicitly selected installed Framework SDK.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT

cmake_minimum_required(VERSION 3.25)
project(UmicomOSBootMedia LANGUAGES C)
include(CTest)
option(UMICOM_OS_BOOT_MEDIA_USE_INSTALLED_SDK "Link the OS console to a qualified installed Framework SDK" OFF)
set(UMICOM_SDK_ROOT "" CACHE PATH "Selected installed Framework SDK prefix")
if(UMICOM_OS_BOOT_MEDIA_USE_INSTALLED_SDK)
    if(NOT IS_ABSOLUTE "${UMICOM_SDK_ROOT}" OR NOT IS_DIRECTORY "${UMICOM_SDK_ROOT}")
        message(FATAL_ERROR "Installed boot-media mode requires an absolute existing UMICOM_SDK_ROOT")
    endif()
    # Do not fall back to repository sources or a different package installation.
    find_package(UmicomFramework 0.9 CONFIG REQUIRED COMPONENTS boot_media platform
        PATHS "${UMICOM_SDK_ROOT}" NO_DEFAULT_PATH)
    foreach(_helper umicom_require_installed_sdk umicom_require_sdk_environment
                    umicom_apply_application_branding umicom_apply_warnings umicom_apply_sanitizers)
        if(NOT COMMAND ${_helper})
            message(FATAL_ERROR "Selected SDK lacks required helper: ${_helper}")
        endif()
    endforeach()
    umicom_require_installed_sdk("${UMICOM_SDK_ROOT}" Umicom::boot_media Umicom::platform)
    umicom_require_sdk_environment(2)
    if(DEFINED UMICOM_OS_FRAMEWORK_SOURCE OR DEFINED UMICOM_BOOT_MEDIA_ENABLE_DEVICE_WRITES)
        message(FATAL_ERROR "Installed mode takes source and device-write policy from the producer SDK; use a separate clean consumer cache without these source-only options")
    endif()
    add_executable(umicom-os-boot-media main.c)
    target_link_libraries(umicom-os-boot-media PRIVATE Umicom::boot_media Umicom::platform)
    set_target_properties(umicom-os-boot-media PROPERTIES
        C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    umicom_apply_warnings(umicom-os-boot-media)
    umicom_apply_sanitizers(umicom-os-boot-media)
    umicom_apply_application_branding(TARGET umicom-os-boot-media
        PRODUCT_NAME "Umicom OS Boot Media" INTERNAL_NAME "umicom-os-boot-media"
        APPLICATION_ID "org.umicom.os.boot-media" VERSION_MAJOR 1 VERSION_MINOR 0 VERSION_PATCH 0)
    if(BUILD_TESTING)
        foreach(_case help help-word self-test missing-command unknown-command
                      unicode-command missing-image duplicate-option unexpected-option)
            add_test(NAME os.boot_media_sdk.${_case}
                COMMAND "${CMAKE_COMMAND}" "-DEXECUTABLE=$<TARGET_FILE:umicom-os-boot-media>"
                    "-DCASE=${_case}" -P "${CMAKE_CURRENT_LIST_DIR}/tests/check_console.cmake")
            set_tests_properties(os.boot_media_sdk.${_case} PROPERTIES
                TIMEOUT 20 LABELS "os;sdk;boot-media;no-device")
        endforeach()
    endif()
else()
    if(UMICOM_SDK_ROOT)
        message(FATAL_ERROR "Set UMICOM_OS_BOOT_MEDIA_USE_INSTALLED_SDK=ON to use UMICOM_SDK_ROOT; select a separate build directory")
    endif()
    # Preserve the existing source composition and its native tools. The package
    # branch above only changes dependency ownership for an explicit consumer.
    set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH
        "Canonical Framework checkout containing native boot-media services")
    if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/boot_media/CMakeLists.txt")
        message(FATAL_ERROR "Select the canonical Framework checkout with the boot-media tools")
    endif()
    add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/boot_media" framework-boot-media)
endif()
