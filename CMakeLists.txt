##############################################################################
# coolfluid cmake build system
##############################################################################
# Command line options for cmake:
# -DCMAKE_BUILD_TYPE=DEBUG     will set the build type
# -G"Visual Studio 9 2008"     will generate for WIN32
# -DCMAKE_INSTALL_PREFIX=DIR   will specify where to install
# -DCF_PLUGIN_DIRS:STRING="/dir/path1;/dir/path2"
##############################################################################
# TODO
#  * test packaging with the build system
#  * try using precompiled headers and add a check to their support
#  * consider removing CF_ENABLE_INTERNAL_DEPS
##############################################################################

##############################################################################
# cmake behavior
##############################################################################

CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 FATAL_ERROR )

##############################################################################
# project definition and versioning
##############################################################################

project ( coolfluid C CXX )

# disallow in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "${PROJECT_NAME} requires an out of source build.\nPlease create a separate build directory and run 'cmake path/to/project [options]' there.")
ENDIF()

# option to skip fortran language
# kernel does not need fortran but some plugins might
# developers that know htat they wont need it may turn it off in systems where is not available
IF(NOT CF_SKIP_FORTRAN)
enable_language ( Fortran OPTIONAL )
ENDIF(NOT CF_SKIP_FORTRAN)

# optionally include personal options ( alternative to passing a cache initialization to cmake)
# developers may create this file and to define options that they don't want on the cache
include ( ${coolfluid_BINARY_DIR}/coolfluid.cmake OPTIONAL )

# include other configurations
set ( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include ( DefineVersions )        # define variables with coolfluid versions

##############################################################################
# cmake setup
##############################################################################

include ( DefineFileExtensions )    # define project files extensions

include ( DefineMacros )            # define our own macros

include ( CTest )                   # include testing support

include ( DefineInstallPaths )      # define where to install the files after building

include ( ResetVariables )          # reset variables used for the build

##############################################################################
# configuration options
##############################################################################

include ( DefineGlobalOptions )     # add user global options

include ( CheckOperatingSystem  )   # check for operating system features
include ( CheckCompilerFeatures )   # check compiler features

include ( DefineBuildModes )        # define how build modes behave ( may override some options )
include ( DefineBuildRules )        # defined default compilation flags and linking rules per architecture

include ( ApplyGlobalOptions )      # take options into effect

LOG ( "" )
##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "General Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

add_subdirectory ( cmake )         # dir with macros to be installed

add_subdirectory ( doc )           # dir with documentation to be installed

##############################################################################
# finding MPI (essential) must be after the GlobalOptions

LOG ( "\n +++ Searching for MPI ..." )
include ( DetectMPI )

##############################################################################
# finding boost (essential)

LOG ( "\n +++ Searching for Boost ..." )
include ( DetectBoost )

##############################################################################
# Find Qt - defines QT_USE_FILE and QT_LIBRARIES used below
find_package( Qt4 4.5.0 COMPONENTS QtCore QtGui QtXml QtNetwork )

##############################################################################
# find non essential packages

# using our find macros
LOG ( "" )
find_package(BlasLapack)      # search for Blas Lapack support
find_package(Metis)           # serial domain decomposition
find_package(Parmetis)        # parallel domain decomposition
find_package(Curl)            # curl downloads files on the fly
find_package(Valgrind)        # valgrind for profiling and memmory leak detection
find_package(GooglePerftools) # dynamic profiler and memory checker

# using cmake find macros

find_package(ZLIB)          # file compression support
LOG ( "ZLIB_FOUND: [${ZLIB_FOUND}]" )
IF ( ZLIB_FOUND )
LOG ( "  ZLIB_INCLUDE_DIRS: [${ZLIB_INCLUDE_DIRS}]" )
LOG ( "  ZLIB_LIBRARIES:    [${ZLIB_LIBRARIES}]" )
ENDIF()

find_package (BZip2)        # file compression support
LOG ( "BZIP2_FOUND: [${BZIP2_FOUND}]" )
IF ( BZIP2_FOUND )
LOG ( "  BZIP2_INCLUDE_DIR:  [${BZIP2_INCLUDE_DIR}]" )
LOG ( "  BZIP2_LIBRARIES:    [${BZIP2_LIBRARIES}]" )
LOG ( "  BZIP2_DEFINITIONS:  [${BZIP2_DEFINITIONS}]" )
LOG ( "  BZIP2_NEED_PREFIX:  [${BZIP2_NEED_PREFIX}]" )
ENDIF()

include ( CheckSvnVersion ) # check for subversion support

LOG ( "" )
##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "Kernel Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################


# add dir with kernel config (automatic generated headers)
add_definitions     ( -DCF_HAVE_CONFIG_H )
include_directories ( ${coolfluid_BINARY_DIR} )
add_subdirectory ( config )

# add dir with kernel sources
include_directories ( ${coolfluid_SOURCE_DIR}/src )
add_subdirectory ( src )

if ( CF_ENABLE_SANDBOX )
  add_subdirectory ( sandbox )
endif()

LOG ( "" )
##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "Plugin's Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

# find plugins
# developer may set several base directories that host multiple plugins each
foreach( EXDIR ${CF_PLUGIN_DIRS} )

  include_directories ( ${EXDIR} )

  LOG ( "" )
  LOG ( "---------------------------------------------------------" )
  LOG ( "PLUGINS DIR [${EXDIR}]")
  LOG ( "---------------------------------------------------------" )

  # find modules in each search dir
  file ( GLOB  FOUND_PLUGIN_DIRS "${EXDIR}/*/CMakeLists.txt")
  list ( SORT  FOUND_PLUGIN_DIRS )

  foreach( ADIR ${FOUND_PLUGIN_DIRS} )

    string (REGEX REPLACE "(/)*CMakeLists.txt" "" aPluginDir ${ADIR})

    if ( IS_DIRECTORY ${aPluginDir} )

      get_filename_component ( aPluginName ${aPluginDir} NAME )

      list ( APPEND CF_PLUGIN_LIST ${aPluginName} )

      LOG ( " + PLUGIN [${aPluginName}]" )

      add_subdirectory    ( ${aPluginDir} plugins/${aPluginName} )

    endif ()

  endforeach()

endforeach()


LOG ( "" )
##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "+ coolfluid configuration summary " )
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################
LOG ( " coolfluid Release     : [${CF_VERSION}]" )
LOG ( " coolfluid Kernel      : [${CF_KERNEL_VERSION}]" )
LOG ( " coolfluid SVN version : [${coolfluid_svnversion}]" )
LOG ( "---------------------------------------------------------" )
LOG ( " CMake Version         : [${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}]" )
LOG ( " CMake Generator       : [${CMAKE_GENERATOR}]" )
LOG ( " Build Type            : [${CMAKE_BUILD_TYPE}]" )
LOG ( " Operating System      : [${CMAKE_SYSTEM}]" )
LOG ( " Memory address        : [${CF_OS_BITS} bits]" )
LOG ( "---------------------------------------------------------" )
LOG ( " Install path          : [${CMAKE_INSTALL_PREFIX}]" )
LOG ( " Install tests         : [${CF_INSTALL_TESTS}]" )
LOG ( "---------------------------------------------------------" )
LOG ( " C compiler            : [${CMAKE_C_COMPILER}]" )
LOG ( " C flags               : [${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}]" )
LOG ( " C++ compiler          : [${CMAKE_CXX_COMPILER}]" )
LOG ( " C++ flags             : [${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}]" )
LOG ( " C++ link flags        : [${CMAKE_CXX_LINK_FLAGS}]" )
IF( NOT CF_SKIP_FORTRAN )
LOG ( " Fortran compiler      : [${CMAKE_Fortran_COMPILER}]" )
LOG ( " Fortran flags         : [${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}]" )
ENDIF()
LOG ( " Shared Linker flags   : [${CMAKE_SHARED_LINKER_FLAGS}]" )

LOG ( "---------------------------------------------------------" )
LOG ( " MPI                   : [${CF_HAVE_MPI}]" )
IF(CF_HAVE_MPI)
LOG ( "    CF_MPIRUN_PROGRAM  : [${CF_MPIRUN_PROGRAM}]" )
LOG ( "    MPI_INCLUDE_PATH   : [${MPI_INCLUDE_PATH}]")
LOG ( "    MPI_LIBRARIES      : [${MPI_LIBRARIES}]")
ENDIF()
LOG ( "---------------------------------------------------------" )
LOG ( " Check internal deps   : [${CF_ENABLE_INTERNAL_DEPS}]")
LOG ( " Log all               : [${CF_ENABLE_LOGALL}]")
LOG ( " Log debug             : [${CF_ENABLE_LOGDEBUG}]")
LOG ( " Assertions            : [${CF_ENABLE_ASSERTIONS}]")
LOG ( " Tracing               : [${CF_ENABLE_TRACE}]")
LOG ( " Static libs           : [${CF_ENABLE_STATIC}]")
LOG ( " Profiling             : [${CF_ENABLE_PROFILING}]")
IF(CF_ENABLE_PROFILING)
LOG ( "    Profiler           : [${CF_PROFILER_TOOL}]")
ENDIF()
LOG ( " Explicit Templates    : [${CF_HAVE_CXX_EXPLICIT_TEMPLATES}]")
LOG ( "---------------------------------------------------------" )
LOG ( " Kernel libs           : [${CF_KERNEL_LIBS}]")
LOG ( " List Plugins          : [${CF_PLUGIN_LIST}]")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
LOG ( "")

##############################################################################
# finalize build
##############################################################################

include  ( ProcessOrphanFiles )    # processes the list of orphan files

include  ( DumpVariables )         # dump relevant variables in the log file

include  ( PrepareCPack  )         # packaging instructions

