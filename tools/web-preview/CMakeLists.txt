# Umicom OS
# File: tools/web-preview/CMakeLists.txt
# PURPOSE: Build a thin branded console against one explicit SDK or reviewed source checkout.
# AUTHOR AND ORGANISATION: Sammy Hegab / Umicom Foundation
# LICENCE: MIT
cmake_minimum_required(VERSION 3.25)
project(UmicomOSWebPreview LANGUAGES C)
include(CTest)
set(UMICOM_SDK_ROOT "" CACHE PATH "Qualified installed Framework SDK prefix")
set(UMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE "" CACHE PATH "Optional reviewed primary Framework source checkout")
if(UMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE)
    if(UMICOM_SDK_ROOT)
        message(FATAL_ERROR "Choose one Framework dependency mode in a separate build directory")
    endif()
    if(NOT IS_ABSOLUTE "${UMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE}" OR
       NOT EXISTS "${UMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE}/include/umicom/web/preview_command.h")
        message(FATAL_ERROR "Source mode requires an absolute reviewed Framework checkout with preview_command.h")
    endif()
    add_subdirectory("${UMICOM_OS_WEB_PREVIEW_FRAMEWORK_SOURCE}" framework)
else()
    if(NOT IS_ABSOLUTE "${UMICOM_SDK_ROOT}" OR
       NOT EXISTS "${UMICOM_SDK_ROOT}/include/umicom/web/preview_command.h")
        message(FATAL_ERROR "Select an absolute installed UMICOM_SDK_ROOT containing the shared preview command")
    endif()
    find_package(UmicomFramework 0.9 CONFIG REQUIRED COMPONENTS web platform
        PATHS "${UMICOM_SDK_ROOT}" NO_DEFAULT_PATH)
    umicom_require_installed_sdk("${UMICOM_SDK_ROOT}" Umicom::web Umicom::platform)
    umicom_require_sdk_environment(2)
endif()
add_executable(umicom-os-web-preview main.c)
target_link_libraries(umicom-os-web-preview PRIVATE Umicom::web Umicom::platform)
set_target_properties(umicom-os-web-preview PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
umicom_apply_warnings(umicom-os-web-preview)
umicom_apply_sanitizers(umicom-os-web-preview)
umicom_apply_application_branding(TARGET umicom-os-web-preview
    PRODUCT_NAME "Umicom OS Web Preview" INTERNAL_NAME "umicom-os-web-preview"
    APPLICATION_ID "org.umicom.os.web-preview" VERSION_MAJOR 1 VERSION_MINOR 0 VERSION_PATCH 0)
if(BUILD_TESTING)
    foreach(case help dry-run unicode relative duplicate unknown)
        add_test(NAME os.web_preview.${case} COMMAND "${CMAKE_COMMAND}"
            "-DTOOL=$<TARGET_FILE:umicom-os-web-preview>" "-DCASE=${case}"
            -P "${CMAKE_CURRENT_LIST_DIR}/tests/check_console.cmake")
        set_tests_properties(os.web_preview.${case} PROPERTIES TIMEOUT 20 LABELS "os;web;sdk;no-network")
    endforeach()
endif()
install(TARGETS umicom-os-web-preview RUNTIME DESTINATION bin)
