#Sets:
# CMATH_LIBRARIES   = the library to link against (RT etc)
# CF3_HAVE_CMATH    = set to true after finding the library

option( CF3_SKIP_CMATH "Skip search for C math library" OFF )

  coolfluid_set_trial_library_path("") # clear library search path

  if( DEFINED CMATH_HOME )
    coolfluid_add_trial_library_path( ${CMATH_HOME}/lib )
  endif()

  find_library(CMATH_LIBRARIES m ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CMATH_LIBRARIES m )

coolfluid_set_package( PACKAGE Cmath
                       DESCRIPTION "C math library"
                       TYPE OPTIONAL
                       PURPOSE " "
                       VARS CMATH_LIBRARIES
                       Quiet)
