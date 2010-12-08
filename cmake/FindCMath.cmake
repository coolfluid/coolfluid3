#Sets:
# CMATH_LIBRARIES      = the library to link against (RT etc)
# CF_HAVE_CMATH        = set to true after finding the library

option( CF_SKIP_CMATH "Skip search for C math library" OFF )

if( NOT CF_SKIP_CMATH )

  coolfluid_set_trial_library_path("") # clear library search path

  if( DEFINED CMATH_HOME )
    coolfluid_add_trial_library_path( ${CMATH_HOME}/lib )
  endif()

  find_library(CMATH_LIBRARIES m ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CMATH_LIBRARIES m )

  if( CMATH_LIBRARIES )
    set(CF_HAVE_CMATH 1 CACHE BOOL "Found C math library")
  else()
    set(CF_HAVE_CMATH 0 CACHE BOOL "C math library not found")
  endif()

else()
    set(CF_HAVE_CMATH 0 CACHE BOOL "Skipped C math library search")
endif()

mark_as_advanced( CMATH_LIBRARIES CF_HAVE_CMATH )

if ( ${CF_HAVE_CMATH} )
    list( APPEND CF_TP_LIBRARIES ${CMATH_LIBRARIES} )
endif()

coolfluid_log( "CF_HAVE_CMATH: [${CF_HAVE_CMATH}]" )
if(CF_HAVE_CMATH)
    coolfluid_log( "  CMATH_LIBRARIES:      [${CMATH_LIBRARIES}]" )
endif()
