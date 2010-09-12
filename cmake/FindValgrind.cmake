# Sets:
# valgrind_INCLUDE_DIR  = where valgrind.h can be found
# CF_HAVE_VALGRIND      = set to true after finding the library

OPTION ( CF_SKIP_VALGRIND "Skip search for Valgrins library" OFF )
if( NOT CF_SKIP_VALGRIND ) # guard for double inclusion

  SET_TRIAL_INCLUDE_PATH ("") # clear include search path
  SET_TRIAL_LIBRARY_PATH ("") # clear library search path

  ADD_TRIAL_INCLUDE_PATH( ${VALGRIND_HOME}/include )
  ADD_TRIAL_INCLUDE_PATH( $ENV{VALGRIND_HOME}/include )

  FIND_PATH(valgrind_INCLUDE_DIR valgrind/valgrind.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  FIND_PATH(valgrind_INCLUDE_DIR valgrind/valgrind.h)

  if( valgrind_INCLUDE_DIR )
    set(CF_HAVE_VALGRIND 1 CACHE BOOL "Found valgrind headers")
  ELSE(valgrind_INCLUDE_DIR )
    set(CF_HAVE_VALGRIND 0 CACHE BOOL "Not fount valgrind headers")
  endif(valgrind_INCLUDE_DIR )

  mark_as_advanced ( valgrind_INCLUDE_DIR CF_HAVE_VALGRIND )

  coolfluid_log( "CF_HAVE_VALGRIND: [${CF_HAVE_VALGRIND}]" )
  if(CF_HAVE_VALGRIND)
    coolfluid_log_file ( "  valgrind_INCLUDE_DIR:  [${valgrind_INCLUDE_DIR}]" )
  endif(CF_HAVE_VALGRIND)

endif( NOT CF_SKIP_VALGRIND )
