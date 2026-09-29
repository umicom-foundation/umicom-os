# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
cmake_minimum_required(VERSION 3.24)
project(UmicomEnterpriseRecoveryHost LANGUAGES C)
include(CTest)
get_filename_component(_framework "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
set(UMICOM_OS_FRAMEWORK_SOURCE "${_framework}" CACHE PATH "Framework checkout")
set(UMICOM_FRAMEWORK_SOURCE "${UMICOM_OS_FRAMEWORK_SOURCE}" CACHE PATH "Framework checkout" FORCE)
add_subdirectory("${UMICOM_FRAMEWORK_SOURCE}/examples/enterprise_recovery" enterprise-recovery)
