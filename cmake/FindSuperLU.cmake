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

option( CF_SKIP_SUPERLU "Skip search for SuperLU library" OFF )

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

coolfluid_log_deps_result( SUPERLU SUPERLU_INCLUDE_DIR SUPERLU_LIBRARIES )
