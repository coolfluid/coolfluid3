# - Try to find Python include dirs and libraries
#
# Usage of this module as follows:
#
#     find_package(PythonDev)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  PYTHON_EXECUTABLE         If this is set to a path to a Python interpreter
#                            then this module attempts to infer the path to
#                            python-config from it
#  PYTHON_CONFIG             Set this variable to the location of python-config
#                            if the module has problems finding the proper
#                            installation path.
#
# Variables defined by this module:
#
#  PYTHONDEV_FOUND           System has Python dev headers/libraries
#  PYTHON_INCLUDE_DIR        The Python include directories.
#  PYTHON_LIBRARIES          The Python libraries and linker flags.

include(FindPackageHandleStandardArgs)

if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE}-config)
    set(PYTHON_CONFIG ${PYTHON_EXECUTABLE}-config CACHE PATH "" FORCE)
else ()
    find_program(PYTHON_CONFIG
        NAMES python-config python-config2.7 python-config2.6 python-config2.6
              python-config2.4 python-config2.3)
endif ()

# The OpenBSD python packages have python-config's that don't reliably
# report linking flags that will work.
if (PYTHON_CONFIG AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    execute_process(COMMAND "${PYTHON_CONFIG}" --ldflags
                    OUTPUT_VARIABLE PYTHON_LIBRARIES
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    execute_process(COMMAND "${PYTHON_CONFIG}" --includes
                    OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)

    string(REGEX REPLACE "^[-I]" "" PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}")
    string(REGEX REPLACE "[ ]-I" " " PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}")
    separate_arguments(PYTHON_INCLUDE_DIR)

    find_package_handle_standard_args(PythonDev DEFAULT_MSG
        PYTHON_CONFIG
        PYTHON_INCLUDE_DIR
        PYTHON_LIBRARIES
    )
else ()
    find_package(PythonLibs)
    if (PYTHON_INCLUDE_PATH AND NOT PYTHON_INCLUDE_DIR)
        set(PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_PATH}")
    endif ()
    find_package_handle_standard_args(PythonDev DEFAULT_MSG
        PYTHON_INCLUDE_DIR
        PYTHON_LIBRARIES
    )
endif ()
