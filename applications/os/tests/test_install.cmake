#-----------------------------------------------------------------------------
# Umicom OS
# File: applications/os/tests/test_install.cmake
# PURPOSE: Verify that product installation contains the built OS programs.
# AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
# LICENCE: MIT
#-----------------------------------------------------------------------------

# Require explicit build-owned paths so a direct invocation cannot fall back to
# an unrelated installation or the current working directory.
foreach(_required IN ITEMS UMICOM_INSTALL_BINARY_DIR UMICOM_INSTALL_TEST_PREFIX
        UMICOM_INSTALL_BINDIR UMICOM_INSTALL_CONSOLE)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing installation fixture input: ${_required}")
    endif()
endforeach()
if(IS_ABSOLUTE "${UMICOM_INSTALL_BINDIR}")
    message(FATAL_ERROR "The installation fixture requires a relative executable directory")
endif()


# A staging path must stay inside this build tree, including after normalising
# dot segments. This prevents a custom install-directory setting from writing
# into a real system prefix while the fixture verifies packaging.
set(_build_root "${UMICOM_INSTALL_BINARY_DIR}")
set(_stage_root "${UMICOM_INSTALL_TEST_PREFIX}")
cmake_path(ABSOLUTE_PATH _build_root NORMALIZE)
cmake_path(ABSOLUTE_PATH _stage_root NORMALIZE)
cmake_path(IS_PREFIX _build_root "${_stage_root}" NORMALIZE _stage_in_build)
if(NOT _stage_in_build OR _stage_root STREQUAL _build_root)
    message(FATAL_ERROR "OS installation staging must be a directory inside the build tree")
endif()
set(_executable_directory "${UMICOM_INSTALL_BINDIR}")
cmake_path(ABSOLUTE_PATH _executable_directory BASE_DIRECTORY "${_stage_root}" NORMALIZE)
cmake_path(IS_PREFIX _stage_root "${_executable_directory}" NORMALIZE _programs_in_stage)
if(NOT _programs_in_stage)
    message(FATAL_ERROR "The executable directory escapes the installation staging prefix")
endif()

# Fail before installation if the build did not produce an expected program.
# An absent executable must not be treated as an optional successful install.
set(_programs "${UMICOM_INSTALL_CONSOLE}")
if(DEFINED UMICOM_INSTALL_SHELL AND NOT "${UMICOM_INSTALL_SHELL}" STREQUAL "")
    list(APPEND _programs "${UMICOM_INSTALL_SHELL}")
endif()
foreach(_program IN LISTS _programs)
    if(NOT EXISTS "${_program}" OR IS_DIRECTORY "${_program}")
        message(FATAL_ERROR "Build the OS program before testing installation: ${_program}")
    endif()
endforeach()

# Only the OS component is staged here. Framework's SDK has its own install
# rules and does not need to be copied for every product packaging regression.
execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${UMICOM_INSTALL_BINARY_DIR}"
        --prefix "${UMICOM_INSTALL_TEST_PREFIX}" --config "${UMICOM_INSTALL_CONFIG}"
        --component UmicomOS
    RESULT_VARIABLE _install_result
    OUTPUT_VARIABLE _install_output ERROR_VARIABLE _install_error TIMEOUT 45)
if(NOT _install_result STREQUAL "0")
    message(FATAL_ERROR "OS installation failed:\n${_install_output}\n${_install_error}")
endif()

# Comparing hashes checks the current binary, not merely a file left behind by
# an earlier build. Keep this assertion when adding further native OS programs.
foreach(_program IN LISTS _programs)
    get_filename_component(_name "${_program}" NAME)
    set(_installed "${UMICOM_INSTALL_TEST_PREFIX}/${UMICOM_INSTALL_BINDIR}/${_name}")
    if(NOT EXISTS "${_installed}" OR IS_DIRECTORY "${_installed}")
        message(FATAL_ERROR "The OS installer omitted ${_name}")
    endif()
    file(SHA256 "${_program}" _built_hash)
    file(SHA256 "${_installed}" _installed_hash)
    if(NOT _built_hash STREQUAL _installed_hash)
        message(FATAL_ERROR "Installed program differs from the build: ${_name}")
    endif()
endforeach()
