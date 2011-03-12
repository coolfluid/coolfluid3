# this module looks for Zoltan library
# it will define the following values
#
# Needs environmental variables
#   SUPERLU_HOME
# Sets
#   SUPERLU_INCLUDE_DIR
#   SUPERLU_LIBRARIES
#   CF_HAVE_SUPERLU
#

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

coolfluid_add_trial_include_path( ${SUPERLU_HOME}/include )
coolfluid_add_trial_include_path( $ENV{SUPERLU_HOME}/include )

find_path( SUPERLU_INCLUDE_DIR superlu PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
find_path( SUPERLU_INCLUDE_DIR superlu )
    
coolfluid_add_trial_library_path(${SUPERLU_HOME}/lib )
coolfluid_add_trial_library_path($ENV{SUPERLU_HOME}/lib)

find_library(SUPERLU_LIBRARIES superlu  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
find_library(SUPERLU_LIBRARIES superlu )

if(SUPERLU_INCLUDE_DIR AND SUPERLU_LIBRARIES)
  set(CF_HAVE_SUPERLU 1)
else()
  set(CF_HAVE_SUPERLU 0)
endif()

mark_as_advanced(
  SUPERLU_INCLUDE_DIR
  SUPERLU_LIBRARIES
  CF_HAVE_SUPERLU
)

if ( ${CF_HAVE_SUPERLU} )
    list( APPEND CF_DEPS_LIBRARIES ${SUPERLU_LIBRARIES} )
endif()

coolfluid_log( "CF_HAVE_SUPERLU: [${CF_HAVE_SUPERLU}]" )
if(CF_HAVE_SUPERLU)
   coolfluid_log( "  SUPERLU_INCLUDE_DIR: [${SUPERLU_INCLUDE_DIR}]" )
   coolfluid_log( "  SUPERLU_LIBRARIES:     [${SUPERLU_LIBRARIES}]" )
else()
   coolfluid_log_file( "  SUPERLU_INCLUDE_DIR: [${SUPERLU_INCLUDE_DIR}]" )
   coolfluid_log_file( "  SUPERLU_LIBRARIES:     [${SUPERLU_LIBRARIES}]" )
endif()
