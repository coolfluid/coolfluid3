# Sets:
# valgrind_INCLUDE_DIR  = where valgrind.h can be found
# CF_HAVE_VALGRIND      = set to true after finding the library

option( CF_SKIP_VALGRIND "Skip search for Valgrins library" OFF )
if( NOT CF_SKIP_VALGRIND ) # guard for double inclusion

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${VALGRIND_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{VALGRIND_HOME}/include )

  find_path(valgrind_INCLUDE_DIR valgrind/valgrind.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(valgrind_INCLUDE_DIR valgrind/valgrind.h)

  if( valgrind_INCLUDE_DIR )
    set(CF_HAVE_VALGRIND 1 CACHE BOOL "Found valgrind headers")
  ELSE(valgrind_INCLUDE_DIR )
    set(CF_HAVE_VALGRIND 0 CACHE BOOL "Not fount valgrind headers")
  endif(valgrind_INCLUDE_DIR )

  mark_as_advanced( valgrind_INCLUDE_DIR CF_HAVE_VALGRIND )

  coolfluid_log( "CF_HAVE_VALGRIND: [${CF_HAVE_VALGRIND}]" )
  if(CF_HAVE_VALGRIND)
    coolfluid_log_file( "  valgrind_INCLUDE_DIR:  [${valgrind_INCLUDE_DIR}]" )
  endif(CF_HAVE_VALGRIND)

endif( NOT CF_SKIP_VALGRIND )
