# Umicom OS | Sammy Hegab, Umicom Foundation | MIT
# Host-side banking laboratory; does not change the guest or duplicate services.
cmake_minimum_required(VERSION 3.24)
project(UmicomBankReviewHost LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework checkout")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/bank_review/CMakeLists.txt")
    message(FATAL_ERROR "Set UMICOM_OS_FRAMEWORK_SOURCE to the updated Framework checkout")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/bank_review" "${CMAKE_CURRENT_BINARY_DIR}/framework-bank-review")
