# this module looks for Zoltan library
# it will define the following values
#
# Needs environmental variables
#   ZOLTAN_HOME
# Sets
#   ZOLTAN_INCLUDE_DIR
#   ZOLTAN_LIBRARIES
#   CF_HAVE_ZOLTAN
#

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

coolfluid_add_trial_include_path( ${ZOLTAN_HOME}/include )
coolfluid_add_trial_include_path( $ENV{ZOLTAN_HOME}/include )

find_path( ZOLTAN_INCLUDE_DIR zoltan.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
find_path( ZOLTAN_INCLUDE_DIR zoltan.h )
    
coolfluid_add_trial_library_path(${ZOLTAN_HOME}/lib )
coolfluid_add_trial_library_path($ENV{ZOLTAN_HOME}/lib)

find_library(ZOLTAN_LIBRARIES zoltan  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
find_library(ZOLTAN_LIBRARIES zoltan )

if( ${CF_HAVE_PARMETIS} )
  list( APPEND ZOLTAN_LIBRARIES ${PARMETIS_LIBRARIES} )
  list( APPEND ZOLTAN_INCLUDE_DIR ${PARMETIS_INCLUDE_DIR} )
endif()

if( ${CF_HAVE_PTSCOTCH} )
  list( APPEND ZOLTAN_LIBRARIES ${PTSCOTCH_LIBRARIES} )
  list( APPEND ZOLTAN_INCLUDE_DIR ${PTSCOTCH_INCLUDE_DIR} )
endif()

if(ZOLTAN_INCLUDE_DIR AND ZOLTAN_LIBRARIES)
  set(CF_HAVE_ZOLTAN 1)
else()
  set(CF_HAVE_ZOLTAN 0)
endif()

mark_as_advanced(
  ZOLTAN_INCLUDE_DIR
  ZOLTAN_LIBRARIES
  CF_HAVE_ZOLTAN
)

coolfluid_log( "CF_HAVE_ZOLTAN: [${CF_HAVE_ZOLTAN}]" )
if(CF_HAVE_ZOLTAN)
   coolfluid_log( "  ZOLTAN_INCLUDE_DIR: [${ZOLTAN_INCLUDE_DIR}]" )
   coolfluid_log( "  ZOLTAN_LIBRARIES:     [${ZOLTAN_LIBRARIES}]" )
else()
   coolfluid_log_file( "  ZOLTAN_INCLUDE_DIR: [${ZOLTAN_INCLUDE_DIR}]" )
   coolfluid_log_file( "  ZOLTAN_LIBRARIES:     [${ZOLTAN_LIBRARIES}]" )
endif()
