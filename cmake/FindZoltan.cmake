# this module looks for Zoltan library
# it will define the following values
#
# Needs environmental variables
#   ZOLTAN_HOME
# Sets
#   ZOLTAN_INCLUDE_DIR
#   ZOLTAN_LIBRARIES
#   CF3_HAVE_ZOLTAN
#
option( CF3_SKIP_ZOLTAN "Skip search for Zoltan library" OFF )

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

coolfluid_add_trial_include_path( ${ZOLTAN_HOME}/include )
coolfluid_add_trial_include_path( $ENV{ZOLTAN_HOME}/include )

coolfluid_add_trial_library_path(${ZOLTAN_HOME}/lib )
coolfluid_add_trial_library_path($ENV{ZOLTAN_HOME}/lib)

# If Zoltan installation happend through CMake and Trilinos, a ZoltanConfig.cmake file exists
# that has information of which third party libraries Zoltan is built with
find_file( ZOLTAN_CONFIG_FILE ZoltanConfig.cmake ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
find_file( ZOLTAN_CONFIG_FILE ZoltanConfig.cmake )

if( ${ZOLTAN_CONFIG_FILE} )

  include( ${ZOLTAN_CONFIG_FILE} )

  list( APPEND ZOLTAN_INCLUDE_DIR ${Zoltan_INCLUDE_DIRS})
  list( APPEND ZOLTAN_INCLUDE_DIR ${Zoltan_TPL_INCLUDE_DIRS})

  find_library(ZOLTAN_LIBRARIES ${Zoltan_LIBRARIES}  PATHS  ${Zoltan_LIBRARY_DIRS}  NO_DEFAULT_PATH)
  find_library(ZOLTAN_LIBRARIES ${Zoltan_LIBRARIES} )

  list( APPEND ZOLTAN_LIBRARIES ${Zoltan_TPL_LIBRARIES} )

else()

  find_path( ZOLTAN_INCLUDE_DIR zoltan.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
  find_path( ZOLTAN_INCLUDE_DIR zoltan.h )

  find_library(ZOLTAN_LIBRARIES zoltan  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
  find_library(ZOLTAN_LIBRARIES zoltan )

  if( ${CF3_HAVE_PARMETIS} )
    list( APPEND ZOLTAN_LIBRARIES ${PARMETIS_LIBRARIES} )
    list( APPEND ZOLTAN_INCLUDE_DIR ${PARMETIS_INCLUDE_DIR} )
  endif()

  if( ${CF3_HAVE_PTSCOTCH} )
    list( APPEND ZOLTAN_LIBRARIES ${PTSCOTCH_LIBRARIES} )
    list( APPEND ZOLTAN_INCLUDE_DIR ${PTSCOTCH_INCLUDE_DIR} )
  endif()

endif()

coolfluid_add_package( PACKAGE Zoltan
                       DESCRIPTION "parallel graph partitioning"
                       URL "http://trilinos.sandia.gov/packages/zoltan"
                       VARS
                       ZOLTAN_INCLUDE_DIR ZOLTAN_LIBRARIES  )
