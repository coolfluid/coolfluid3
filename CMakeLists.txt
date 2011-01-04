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
#  * try using precompiled headers and add a check to their support
##############################################################################

##############################################################################
# cmake behavior
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

##############################################################################
# project definition and versioning
##############################################################################

project( coolfluid C CXX )

# disallow in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "${PROJECT_NAME} requires an out of source build.\nPlease create a separate build directory and run 'cmake path/to/project [options]' there.")
endif()

# option to skip fortran language
# kernel does not need fortran but some plugins might
# developers that know htat they wont need it may turn it off in systems where is not available
if(NOT CF_SKIP_FORTRAN)
  enable_language( Fortran OPTIONAL )
else()
  set( CF_SKIP_FORTRAN ON CACHE BOOL "Skipping fortran language" FORCE )
endif(NOT CF_SKIP_FORTRAN)

# optionally include personal options ( alternative to passing a cache initialization to cmake)
# developers may create this file and to define options that they don't want on the cache
include( ${coolfluid_BINARY_DIR}/coolfluid.cmake OPTIONAL )

# include other configurations
set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include( DefineVersions )        # define variables with coolfluid versions

##############################################################################
# cmake setup
##############################################################################

include( DefineFileExtensions )    # define project files extensions

include( DefineMacros )            # define our own macros

include( CTest )                   # include testing support

include( DefineInstallPaths )      # define where to install the files after building

include( ResetVariables )          # reset variables used for the build and set DEPS_ROOT if needed

##############################################################################
# configuration options
##############################################################################

include( DefineGlobalOptions )     # add user global options

include( CheckOperatingSystem  )   # check for operating system features

include( CheckCompilerFeatures )   # check compiler features

include( DefineBuildModes )        # define how build modes behave ( may override some options )

include( DefineBuildRules )        # defined default compilation flags and linking rules per architecture

include( ApplyGlobalOptions )      # take options into effect

coolfluid_log( "" )
##############################################################################
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( "General Configuration ")
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

add_subdirectory( cmake )         # dir with macros to be installed

add_subdirectory( tools )         # dir with tools and scripts

add_subdirectory( doc )           # dir with documentation to be installed

include( DetectSoftware )         # find sofwtare dependencies

include( CheckSourceRevision )        # check for subversion support

coolfluid_log( "" )
##############################################################################
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( "Kernel Configuration ")
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

# dir includes
include_directories( ${coolfluid_BINARY_DIR} )
include_directories( ${coolfluid_SOURCE_DIR}/include )
include_directories( ${coolfluid_SOURCE_DIR}/src )
include_directories( ${coolfluid_SOURCE_DIR}/plugins )

# add dir with kernel config (automatic generated headers)
add_subdirectory( config )

# define the resources directory
add_subdirectory( resources )

# add dir with external kernel dependencies but locally included
add_subdirectory( include )

# add dir with kernel sources
add_subdirectory( src )

# add dir with unit tests
add_subdirectory( test )

# sandbox is where we play around before putting stuff in the released code, off by default
if( CF_ENABLE_SANDBOX )
  add_subdirectory( sandbox EXCLUDE_FROM_ALL )
endif()

coolfluid_log( "" )
##############################################################################
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( "Plugin's Configuration ")
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

# add dir with kernel plugins
# these are independent source trees that happen to be distributed with the kernel for convinience
add_subdirectory( plugins )

# find plugins
# developer may set several base directories that host multiple plugins each
foreach( EXDIR ${CF_PLUGIN_DIRS} )

  include_directories( ${EXDIR} )

  coolfluid_log( "" )
  coolfluid_log( "---------------------------------------------------------" )
  coolfluid_log( "PLUGINS DIR [${EXDIR}]")
  coolfluid_log( "---------------------------------------------------------" )

  # find modules in each search dir
  file( GLOB  FOUND_PLUGIN_DIRS "${EXDIR}/*/CMakeLists.txt")
  list( SORT  FOUND_PLUGIN_DIRS )

  foreach( ADIR ${FOUND_PLUGIN_DIRS} )

    string(REGEX REPLACE "(/)*CMakeLists.txt" "" aPluginDir ${ADIR})

    if( IS_DIRECTORY ${aPluginDir} )

      get_filename_component( aPluginName ${aPluginDir} NAME )

      coolfluid_define_plugin( ${aPluginName} ${aPluginDir} )

      add_subdirectory( ${aPluginDir} plugins/${aPluginName} )

    endif()

  endforeach()

endforeach()


coolfluid_log( "" )

coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( "+ coolfluid configuration summary " )
coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
coolfluid_log( " coolfluid Release     : [${CF_VERSION}]" )
coolfluid_log( " coolfluid Kernel      : [${CF_KERNEL_VERSION}]" )
coolfluid_log( " coolfluid SVN version : [${coolfluid_svnversion}]" )
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " CMake Version         : [${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}]" )
coolfluid_log( " CMake Generator       : [${CMAKE_GENERATOR}]" )
coolfluid_log( " build type            : [${CMAKE_BUILD_TYPE}]" )
coolfluid_log( " operating system      : [${CMAKE_SYSTEM}]" )
coolfluid_log( " memory address        : [${CF_OS_BITS} bits]" )
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " Install path          : [${CMAKE_INSTALL_PREFIX}]" )
coolfluid_log( " Install tests         : [${CF_INSTALL_TESTS}]" )
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " C compiler            : [${CMAKE_C_COMPILER}]" )
coolfluid_log( " C flags               : [${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}]" )
coolfluid_log( " C++ compiler          : [${CMAKE_CXX_COMPILER}]" )
coolfluid_log( " C++ flags             : [${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}]" )
coolfluid_log( " C++ link flags        : [${CMAKE_CXX_LINK_FLAGS}]" )
if( NOT CF_SKIP_FORTRAN )
  coolfluid_log( " Fortran compiler      : [${CMAKE_Fortran_COMPILER}]" )
  coolfluid_log( " Fortran flags         : [${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}]" )
endif()
coolfluid_log( " common linker flags   : [${LINK_FLAGS}]" )
coolfluid_log( " shared linker flags   : [${CMAKE_SHARED_LINKER_FLAGS}]" )
coolfluid_log( " link libraries        : [${LINK_LIBRARIES}]" )

coolfluid_log( "---------------------------------------------------------" )
if(CF_MPIRUN_PROGRAM)
  coolfluid_log( "    mpirun  : [${CF_MPIRUN_PROGRAM}]" )
endif()
if( NOT CF_MPI_COMPILER_AVAILABLE )
  coolfluid_log( "    mpi include        : [${MPI_INCLUDE_PATH}]")
  coolfluid_log( "    mpi libraries      : [${MPI_LIBRARIES}]")
endif()
coolfluid_log( "---------------------------------------------------------" )
coolfluid_log( " GUI                   : [${CF_ENABLE_GUI}]")
coolfluid_log( " Sandbox               : [${CF_ENABLE_SANDBOX}]")
coolfluid_log( " Log all               : [${CF_ENABLE_LOGALL}]")
coolfluid_log( " Log debug             : [${CF_ENABLE_LOGDEBUG}]")
coolfluid_log( " Assertions            : [${CF_ENABLE_ASSERTIONS}]")
coolfluid_log( " Tracing               : [${CF_ENABLE_TRACE}]")
coolfluid_log( " Static libs           : [${CF_ENABLE_STATIC}]")
coolfluid_log( " Code coverage         : [${CF_ENABLE_CODECOVERAGE}]")
if(CF_ENABLE_CODECOVERAGE)
  coolfluid_log( "    Coverage tool      : [${CTEST_COVERAGE_COMMAND}]")
endif()
coolfluid_log( " Profiling             : [${CF_ENABLE_PROFILING}]")
if(CF_ENABLE_PROFILING)
  coolfluid_log( "    Profiler           : [${CF_PROFILER_TOOL}]")
endif()
coolfluid_log( " Explicit Templates    : [${CF_HAVE_CXX_EXPLICIT_TEMPLATES}]")
coolfluid_log( "---------------------------------------------------------" )
# coolfluid_log( " Kernel libs           : [${CF_KERNEL_LIBS}]")
# coolfluid_log( " List Plugins          : [${CF_PLUGIN_LIST}]")
# coolfluid_log( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
coolfluid_log( "")

##############################################################################
# finalize build
##############################################################################

if(CF_CHECK_ORPHAN_FILES)
  include( ProcessOrphanFiles )    # processes the list of orphan files
endif()

include( DumpVariables )         # dump relevant variables in the log file

include( PrepareCPack  )         # packaging instructions
