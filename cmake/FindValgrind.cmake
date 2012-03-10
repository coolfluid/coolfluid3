# Sets:
# VALGRIND_INCLUDE_DIRS  = where valgrind.h can be found
# CF3_HAVE_VALGRIND      = set to true after finding the library

option( CF3_SKIP_VALGRIND "Skip search for Valgrind library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${VALGRIND_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{VALGRIND_HOME}/include )

  find_path(VALGRIND_INCLUDE_DIRS valgrind/valgrind.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(VALGRIND_INCLUDE_DIRS valgrind/valgrind.h)

coolfluid_set_package( PACKAGE Valgrind
                       DESCRIPTION "code analysis and profiling"
                       URL "http://valgrind.org"
                       TYPE OPTIONAL
                       VARS VALGRIND_INCLUDE_DIRS
                       QUIET
                     )

