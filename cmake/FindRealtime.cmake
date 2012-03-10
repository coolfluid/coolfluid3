#Sets:
# RT_LIBRARIES      = the library to link against (RT etc)
# CF3_HAVE_RT        = set to true after finding the library

option( CF3_SKIP_RT "Skip search for RT library" OFF )

  coolfluid_set_trial_library_path("") # clear library search path
  if( DEFINED RT_HOME )
    coolfluid_add_trial_library_path( ${RT_HOME}/lib )
  endif()

  find_library(RT_LIBRARIES rt ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(RT_LIBRARIES rt )

coolfluid_set_package( PACKAGE Realtime
                       DESCRIPTION "linux real time library"
                       TYPE OPTIONAL
                       VARS
                       RT_LIBRARIES )
