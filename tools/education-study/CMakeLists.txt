# Umicom Foundation | Sammy Hegab | MIT
# OS host composition only. The kernel does not depend on Education services.
cmake_minimum_required(VERSION 3.25)
project(UmicomEducationStudyHost LANGUAGES C)
include(CTest)
set(UMICOM_OS_FRAMEWORK_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../framework" CACHE PATH "Canonical Framework source")
if(NOT EXISTS "${UMICOM_OS_FRAMEWORK_SOURCE}/examples/education_study/CMakeLists.txt")
    message(FATAL_ERROR "Select the Framework checkout containing the education-study SDK")
endif()
add_subdirectory("${UMICOM_OS_FRAMEWORK_SOURCE}/examples/education_study" study)
