coolfluid_log_file("-----------------------------------")
coolfluid_log_file("Generic CMake Variables" )
coolfluid_log_file("-----------------------------------")

# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise
# this is the top level directory of your build tree
coolfluid_log_file( "CMAKE_BINARY_DIR:         ${CMAKE_BINARY_DIR}"  )

# if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this
# is the directory where the compiled or generated files from the current CMakeLists.txt will go to
coolfluid_log_file( "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}" )

# this is the directory, from which cmake was started, i.e. the top level source directory
coolfluid_log_file( "CMAKE_SOURCE_DIR:         ${CMAKE_SOURCE_DIR}" )

# this is the directory where the currently processed CMakeLists.txt is located in
coolfluid_log_file( "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}" )

# contains the full path to the top level directory of your build tree
coolfluid_log_file( "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}" )

# contains the full path to the root of your project source directory,
# i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command
coolfluid_log_file( "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}" )

# set this variable to specify a common place where CMake should put all executable files
# (instead of CMAKE_CURRENT_BINARY_DIR)
coolfluid_log_file( "EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}" )

# set this variable to specify a common place where CMake should put all libraries
# (instead of CMAKE_CURRENT_BINARY_DIR)
coolfluid_log_file( "LIBRARY_OUTPUT_PATH:      ${LIBRARY_OUTPUT_PATH}" )

# tell CMake to search first in directories listed in CMAKE_MODULE_PATH
# when you use find_package() or include()
coolfluid_log_file( "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}" )

# this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake)
coolfluid_log_file( "CMAKE_COMMAND: ${CMAKE_COMMAND}" )

# this is the CMake installation directory
coolfluid_log_file( "CMAKE_ROOT: ${CMAKE_ROOT}" )

# this is the filename including the complete path of the file where this variable is used.
coolfluid_log_file( "CMAKE_CURRENT_LIST_FILE: ${CMAKE_CURRENT_LIST_FILE}" )

# this is linenumber where the variable is used
coolfluid_log_file( "CMAKE_CURRENT_LIST_LINE: ${CMAKE_CURRENT_LIST_LINE}" )

# this is used when searching for include files e.g. using the find_path() command.
coolfluid_log_file( "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}" )

# this is used when searching for libraries e.g. using the find_library() command.
coolfluid_log_file( "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}" )

# the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1"
coolfluid_log_file( "CMAKE_SYSTEM: ${CMAKE_SYSTEM}" )

# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
coolfluid_log_file( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )

# only the version part of CMAKE_SYSTEM
coolfluid_log_file( "CMAKE_SYSTEM_VERSION: ${CMAKE_SYSTEM_VERSION}" )

# the processor name e.g. "Intel(R) Pentium(R) M processor 2.00GHz"
coolfluid_log_file( "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )

# is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
coolfluid_log_file( "UNIX: ${UNIX}" )
# is TRUE on Windows, including CygWin
coolfluid_log_file( "WIN32: ${WIN32}" )
# is TRUE on Apple OS X
coolfluid_log_file( "APPLE: ${APPLE}" )
# is TRUE when using the MinGW compiler in Windows
coolfluid_log_file( "MINGW: ${MINGW}" )
# is TRUE on Windows when using the CygWin version of cmake
coolfluid_log_file( "CYGWIN: ${CYGWIN}" )
# is TRUE on Windows when using a Borland compiler
coolfluid_log_file( "BORLAND: ${BORLAND}" )
# Microsoft compiler
coolfluid_log_file( "MSVC:     ${MSVC}" )
coolfluid_log_file( "MSVC_IDE: ${MSVC_IDE}" )
coolfluid_log_file( "MSVC60:   ${MSVC60}" )
coolfluid_log_file( "MSVC70:   ${MSVC70}" )
coolfluid_log_file( "MSVC71:   ${MSVC71}" )
coolfluid_log_file( "MSVC80:   ${MSVC80}" )
coolfluid_log_file( "MSVC90:   ${MSVC90}" )
coolfluid_log_file( "CMAKE_COMPILER_2005: ${CMAKE_COMPILER_2005}" )

# set this to true if you don't want to rebuild the object files if the rules have changed,
# but not the actual source files or headers (e.g. if you changed the some compiler switches)
coolfluid_log_file( "CMAKE_SKIP_RULE_DEPENDENCY: ${CMAKE_SKIP_RULE_DEPENDENCY}" )

# since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing.
# If you don't like this, set this one to true.
coolfluid_log_file( "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY: ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY}" )

# If set, runtime paths are not added when using shared libraries. Default it is set to OFF
coolfluid_log_file( "CMAKE_SKIP_RPATH: ${CMAKE_SKIP_RPATH}" )

# set this to true if you are using makefiles and want to see the full compile and link
# commands instead of only the shortened ones
coolfluid_log_file( "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}" )

# this will cause CMake to not put in the rules that re-run CMake. This might be useful if
# you want to use the generated build files on another machine.
coolfluid_log_file( "CMAKE_SUPPRESS_REGENERATION: ${CMAKE_SUPPRESS_REGENERATION}" )


# A simple way to get switches to the compiler is to use add_definitions().
# But there are also two variables exactly for this purpose:

# the compiler flags for compiling C sources
coolfluid_log_file( "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}" )

# the compiler flags for compiling C++ sources
coolfluid_log_file( "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}" )


# Choose the type of build.  Example: set(CMAKE_BUILD_TYPE Debug)
coolfluid_log_file( "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )

# if this is set to ON, then all libraries are built as shared libraries by default.
coolfluid_log_file( "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}" )

# the compiler used for C files
coolfluid_log_file( "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}" )

# the compiler used for C++ files
coolfluid_log_file( "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}" )

# if the compiler is a variant of gcc, this should be set to 1
coolfluid_log_file( "CMAKE_COMPILER_IS_GNUCC: ${CMAKE_COMPILER_IS_GNUCC}" )

# if the compiler is a variant of g++, this should be set to 1
coolfluid_log_file( "CMAKE_COMPILER_IS_GNUCXX : ${CMAKE_COMPILER_IS_GNUCXX}" )

# the tools for creating libraries
coolfluid_log_file( "CMAKE_AR: ${CMAKE_AR}" )
coolfluid_log_file( "CMAKE_RANLIB: ${CMAKE_RANLIB}" )

coolfluid_log_file( "CMAKE_SHARED_LINKER_FLAGS: [${CMAKE_SHARED_LINKER_FLAGS}}]")
coolfluid_log_file( "CMAKE_MODULE_LINKER_FLAGS: [${CMAKE_MODULE_LINKER_FLAGS}}]")

coolfluid_log_file( "CMAKE_CXX_COMPILE_OBJECT:        [${CMAKE_CXX_COMPILE_OBJECT}]" )
coolfluid_log_file( "CMAKE_CXX_CREATE_SHARED_MODULE:  [${CMAKE_CXX_CREATE_SHARED_MODULE}]" )
coolfluid_log_file( "CMAKE_CXX_CREATE_SHARED_LIBRARY: [${CMAKE_CXX_CREATE_SHARED_LIBRARY}]" )
coolfluid_log_file( "CMAKE_CXX_CREATE_STATIC_LIBRARY: [${CMAKE_CXX_CREATE_STATIC_LIBRARY}]" )
coolfluid_log_file( "CMAKE_CXX_LINK_EXECUTABLE:       [${CMAKE_CXX_LINK_EXECUTABLE}]" )

coolfluid_log_file( "-----------------------------------")

