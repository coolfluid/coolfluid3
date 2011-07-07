#Sets:
# PTHREAD_LIBRARIES      = the library to link against (RT etc)
# CF_HAVE_PTHREAD        = set to true after finding the library

option( CF_SKIP_PTHREAD "Skip search for pthread library" OFF )

  coolfluid_set_trial_library_path("") # clear library search path
  if( DEFINED PTHREAD_HOME )
    coolfluid_add_trial_library_path( ${PTHREAD_HOME}/lib )
  endif()

  find_library(PTHREAD_LIBRARIES pthread ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTHREAD_LIBRARIES pthread )

coolfluid_add_package( PACKAGE PThread DESCRIPTION "Posix Threads"
                       VARS
                       PTHREAD_LIBRARIES )
