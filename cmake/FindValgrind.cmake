# Sets:
# valgrind_INCLUDE_DIR  = where valgrind.h can be found
# CF_HAVE_VALGRIND      = set to true after finding the library

option( CF_SKIP_VALGRIND "Skip search for Valgrins library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${VALGRIND_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{VALGRIND_HOME}/include )

  find_path(VALGRIND_INCLUDE_DIR valgrind/valgrind.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(VALGRIND_INCLUDE_DIR valgrind/valgrind.h)

coolfluid_log_deps_result( VALGRIND VALGRIND_INCLUDE_DIR  )

