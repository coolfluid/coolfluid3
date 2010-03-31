##############################################################################
# coolfluid cmake build system
##############################################################################
# TODO
#  * packaging with the build system
#  * check precompiled headers
##############################################################################
# Command line options for cmake:
# -DCMAKE_BUILD_TYPE=DEBUG     will set the build type
# -DCF_SKIP_FORTRAN=:BOOL=ON   will not test for fortran language
# -DCMAKE_SKIP_RPATH:BOOL=ON   will remove rpath
# -G"Visual Studio 9 2008"     will generate for WIN32
# -DCMAKE_INSTALL_PREFIX=DIR   will specify where to install
# -DCF_PLUGINS_SEARCH_DIRS:STRING="/dir/path1;/dir/path2"
##############################################################################

##############################################################################
# cmake behavior
##############################################################################

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 FATAL_ERROR )

##############################################################################
# project definition and versioning
##############################################################################

project ( coolfluid C CXX )

# disallow in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "coolfluid requires an out of source build.\nPlease create a separate build directory and run 'cmake path_to_coolfluid [options]' there.")
ENDIF()

# option to skip fortran language
# kernel does not need fortran but some plugins might
# developers that know htat they wont need it may turn it off in systems where is not available 
enable_language ( Fortran OPTIONAL )

# optionally include personal options ( alternative to passing a cache initialization to cmake)
# developers may create this file and to define options that they don't want on the cache
include ( ${coolfluid_BINARY_DIR}/coolfluid.cmake OPTIONAL )

# include other configurations
set ( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include ( DefineVersions )        # define variables with coolfluid versions

##############################################################################
# cmake setup
##############################################################################

include ( DefineFileExtensions )   # define project files extensions

include ( DefineMacros )           # define our own macros

include ( CTest )                  # include testing support

include ( DefineInstallPaths )     # define where to install the files after building

##############################################################################
# configuration options
##############################################################################

include ( DefineGlobalOptions )     # add user global options

include ( CheckOperatingSystem  )   # check for operating system features
include ( CheckCompilerFeatures )   # check compiler features

include ( DefineBuildModes )        # define how build modes behave ( may override some options )
include ( DefineBuildRules )        # defined default compilation flags and linking rules per architecture

include ( ApplyGlobalOptions )      # take options into effect

##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "General Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

add_subdirectory ( cmake )         # dir with macros to be installed
add_subdirectory ( doc )           # dir with documentation to be installed

##############################################################################
# finding boost (essential)
##############################################################################

# find boost package (essential)
# SET ( Boost_DEBUG 1 )  # to debug boost search
SET ( Boost_USE_STATIC_LIBS ${CF_ENABLE_STATIC} )
SET ( Boost_USE_MULTITHREAD ON  )
# find based on minimal version defined below
SET ( Boost_FIND_VERSION        ON   )
SET ( Boost_FIND_VERSION_MAJOR  "1"  )
SET ( Boost_FIND_VERSION_MINOR  "42" )
SET ( Boost_FIND_VERSION_PATCH  "0"  )
# older cmakes dont have these versions
SET ( Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" )
# components to search for
LIST ( APPEND CF_Boost_COMPONENTS thread filesystem system regex unit_test_framework )

find_package( Boost COMPONENTS ${CF_Boost_COMPONENTS} )

LOG ( "Boost include path [${Boost_INCLUDE_DIR}]" )
LOG ( "Boost lib version  [${Boost_LIB_VERSION}]" )
LOG ( "Boost libraries    [${Boost_LIBRARIES}]"   )

# if not found give more information
IF ( NOT Boost_FOUND )
  LOG ( ${Boost_ERROR_REASON} )
  MESSAGE ( FATAL_ERROR "Boost is required to compile coolfluid Kernel" )
ENDIF()
# add boost include path
INCLUDE_DIRECTORIES ( ${Boost_INCLUDE_DIR} )
SET ( CF_Boost_LIBRARIES ${Boost_LIBRARIES} )

##############################################################################
# find non essential packages
##############################################################################

# using our find macros
find_package(BlasLapack)    # search for Blas Lapack support
find_package(Metis)         # serial domain decomposition
find_package(Parmetis)      # parallel domain decomposition
find_package(Curl)          # curl downloads files on the fly
find_package(Valgrind)      # valgrind for profiling and memmory leak detection

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

##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "Kernel Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

# include the generated config file
add_definitions     ( -DCF_HAVE_CONFIG_H )
include_directories ( ${coolfluid_BINARY_DIR} )

# if mpi was found add it to the include path if needed
if ( CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER )
  include_directories ( ${MPI_INCLUDE_PATH} )
endif ()

add_subdirectory ( src )        # build the kernel sources

add_subdirectory ( config )     # finalize kernel configuration 

# install global configuration headers
install ( FILES ${coolfluid_BINARY_DIR}/coolfluid_config.h
                ${coolfluid_BINARY_DIR}/coolfluid_svnversion.h
          DESTINATION
                ${CF_INSTALL_INCLUDE_DIR} )

##############################################################################
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
LOG ( "Plugin's Configuration ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" )
##############################################################################

# find plugins
# developer may set several base directories that host multiple plugins each
foreach( EXDIR ${CF_PLUGIN_SEARCH_DIRS} )

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

      LOG ( " + PLUGIN [${aPluginName}]")
      
      add_subdirectory    ( ${aPluginDir} plugins/${aPluginName} )
      
    endif ()

  endforeach()

endforeach()


##############################################################################
# print summary
##############################################################################

LOG ( "")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
LOG ( "+ coolfluid configuration summary ")
LOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
LOG ( " coolfluid Release     : [${CF_VERSION}]")
LOG ( " coolfluid Kernel      : [${CF_KERNEL_VERSION}]")
LOG ( " coolfluid SVN version : [${coolfluid_svnversion}]")
LOG ( "---------------------------------------------------------" )
LOG ( " CMake Version         : [${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}]")
LOG ( " CMake Generator       : [${CMAKE_GENERATOR}]")
LOG ( " Build Type            : [${CMAKE_BUILD_TYPE}]")
LOG ( " Operating System      : [${CMAKE_SYSTEM}]")
LOG ( " Memory address        : [${CF_OS_BITS} bits]")
LOG ( "---------------------------------------------------------" )
LOG ( " Install path          : [${CMAKE_INSTALL_PATH}]")
LOG ( "---------------------------------------------------------" )
LOG ( " C compiler            : [${CMAKE_C_COMPILER}]")
LOG ( " C flags               : [${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}]")
LOG ( " C++ compiler          : [${CMAKE_CXX_COMPILER}]")
LOG ( " C++ flags             : [${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}]")
LOG ( " C++ link flags        : [${CMAKE_CXX_LINK_FLAGS}]")
IF( NOT CF_SKIP_FORTRAN )
LOG ( " Fortran compiler      : [${CMAKE_Fortran_COMPILER}]")
LOG ( " Fortran flags         : [${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}]")
ENDIF()
LOG ( " Shared Linker flags   : [${CMAKE_SHARED_LINKER_FLAGS}]")

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

include  ( ProcessOrphanFiles ) # processes the list of orphan files

include  ( DumpVariables )      # dump relevant variables in the log file

include  ( PrepareCPack  )      # packaging instructions

