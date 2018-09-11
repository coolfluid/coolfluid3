# coolfluid cmake build system
#
# Command line options for cmake:
# -DCMAKE_BUILD_TYPE=DEBUG will set the build type
# -DCF3_SKIP_FORTRAN=:BOOL=ON will not test for fortran language
# -G"Visual Studio 9 2008" will generate for WIN32
# -DCMAKE_INSTALL_PREFIX=DIR will specify where to install
# -DCF3_PLUGIN_DIRS:STRING="/dir/path1;/dir/path2"
#
# TODO
#
# * try using precompiled headers and add a check to their support
#
# * implement and coolfluid3_add_executable
# * add license to cmake files

##############################################################################
# cmake behavior
##############################################################################

cmake_minimum_required( VERSION 3.10 FATAL_ERROR )

cmake_policy( VERSION 3.10 )


##############################################################################
# project definition and versioning
##############################################################################

project( coolfluid C CXX Fortran )

# disallow in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "${PROJECT_NAME} requires an out of source build.\nPlease create a separate build directory and run 'cmake path/to/project [options]' there.")
endif()

# option to skip fortran language
# kernel does not need fortran but some plugins might
# developers that know that they wont need it may turn it off in systems where is not available
if( CF3_ENABLE_FORTRAN )
  enable_language( Fortran OPTIONAL )
else()
  set( CF3_ENABLE_FORTRAN OFF CACHE BOOL "Skipping fortran language" FORCE )
endif()

# optionally include personal options ( alternative to passing a cache initialization to cmake)
# developers may create this file and to define options that they don't want on the cache
include( ${coolfluid_BINARY_DIR}/coolfluid.cmake OPTIONAL )

# include other configurations
set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

##############################################################################
message( STATUS "[configuring ${PROJECT_NAME}]")
##############################################################################

include( DefineVersions ) # define variables with coolfluid versions

include( DefineMacros ) # define our own macros

include( DefineFileExtensions ) # define project files extensions

include( CTest ) # include testing support

include( DefineInstallPaths ) # define where to install the files after building

include( ResetVariables ) # reset variables used for the build and set DEPS_ROOT if needed

coolfluid_log("")

##############################################################################
coolfluid_log(" setting build options ")
##############################################################################


include( DefineGlobalOptions ) # add user global options

include( CheckOperatingSystem ) # check for operating system features

include( CheckCompilerFeatures ) # check compiler features

include( DefineBuildModes ) # define how build modes behave ( may override some options )

include( DefineBuildRules ) # defined default compilation flags and linking rules per architecture

include( ApplyGlobalOptions ) # take options into effect

coolfluid_log("")

##############################################################################
coolfluid_log(" generic configuration ")
##############################################################################

add_subdirectory( cmake ) # dir with macros to be installed

add_subdirectory( tools ) # dir with tools and scripts

add_subdirectory( doc ) # dir with documentation to be installed

include( DetectSoftware ) # find sofwtare dependencies

include( CheckSourceRevision ) # check for subversion support

coolfluid_log("")

##############################################################################
coolfluid_log(" kernel configuration ")
##############################################################################

# dir includes
include_directories( ${coolfluid_BINARY_DIR} )
include_directories( ${coolfluid_SOURCE_DIR} )
include_directories( ${coolfluid_SOURCE_DIR}/cf3 )
include_directories( ${coolfluid_SOURCE_DIR}/include )
include_directories( ${coolfluid_SOURCE_DIR}/plugins )

# add dir with kernel config (automatic generated headers)
add_subdirectory( config )

# define the resources directory
add_subdirectory( resources )

# add dir with external kernel dependencies but locally included
add_subdirectory( include )

# add dir with kernel sources
add_subdirectory( cf3 )

# add dir with unit tests
add_subdirectory( test )

# sandbox is where we play around before putting stuff in the released code, off by default
if( CF3_ENABLE_SANDBOX )
  add_subdirectory( sandbox EXCLUDE_FROM_ALL )
endif()

coolfluid_log("")

##############################################################################
coolfluid_log(" plugin configuration ")
##############################################################################

# add dir with kernel plugins
# these are independent source trees that happen to be distributed with the kernel for convinience
add_subdirectory( plugins )

# find plugins
# developer may set several base directories that host multiple plugins each

macro( subdirlist result curdir )
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      set(dirlist ${dirlist} ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

foreach( EXDIR ${CF3_PLUGIN_DIRS} )

  coolfluid_log_file( "---------------------------------------------------------" )
  coolfluid_log_file( "PLUGINS DIR [${EXDIR}]")
  coolfluid_log_file( "---------------------------------------------------------" )

  subdirlist(SUBDIRS ${EXDIR})

  foreach( subdir_name ${SUBDIRS} )
    set( subdir "${EXDIR}/${subdir_name}" )
    file( GLOB FOUND_PLUGIN_DIR "${subdir}/CMakeLists.txt" )
    if( FOUND_PLUGIN_DIR )
      get_filename_component( plugin_name ${subdir} NAME )
      add_subdirectory( ${subdir} plugins/${plugin_name} )
    else()
      subdirlist( SUBSUBDIRS ${subdir} )
      foreach( subsubdir_name ${SUBSUBDIRS} )
        set( subsubdir "${subdir}/${subsubdir_name}" )
        file( GLOB FOUND_PLUGIN_DIR "${subsubdir}/CMakeLists.txt" )
        if( FOUND_PLUGIN_DIR )
          get_filename_component( plugin_name ${subsubdir} NAME )
          add_subdirectory( ${subsubdir} plugins/${plugin_name} )
        endif()
      endforeach()
    endif()
  endforeach()
endforeach()

coolfluid_log("")

##############################################################################
# summary
##############################################################################

include( PrintSummary )

##############################################################################
# finalize build
##############################################################################

if(CF3_CHECK_ORPHAN_FILES)
  include( ProcessOrphanFiles ) # processes the list of orphan files
endif()

include( DumpVariables ) # dump relevant variables in the log file

include( PrepareCPack ) # packaging instructions

coolfluid_log( "Full configuration log: ${PROJECT_LOG_FILE}" )
