# Sets:
# GOOGLEPERFTOOLS_INCLUDE_DIRS  = where google/profiler.h can be found
# GOOGLEPERFTOOLS_LIBRARIES      = the library to link against
# CF3_HAVE_GOOGLEPERFTOOLS      = set to true after finding the library

option( CF3_SKIP_GOOGLEPERFTOOLS "Skip search for google-perftools" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${GOOGLEPERFTOOLS_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{GOOGLEPERFTOOLS_ROOT}/include )

  find_path(GOOGLEPERFTOOLS_INCLUDE_DIRS google/profiler.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(GOOGLEPERFTOOLS_INCLUDE_DIRS google/profiler.h )

  coolfluid_add_trial_library_path(    ${GOOGLEPERFTOOLS_ROOT}/lib )
  coolfluid_add_trial_library_path( $ENV{GOOGLEPERFTOOLS_ROOT}/lib )

  find_library(GOOGLEPERFTOOLS_PROFILER_LIBRARY profiler ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(GOOGLEPERFTOOLS_PROFILER_LIBRARY profiler )

  find_library(GOOGLEPERFTOOLS_TCMALLOC_LIBRARY tcmalloc ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(GOOGLEPERFTOOLS_TCMALLOC_LIBRARY tcmalloc )

  set( GOOGLEPERFTOOLS_LIBRARIES ${GOOGLEPERFTOOLS_PROFILER_LIBRARY} ${GOOGLEPERFTOOLS_TCMALLOC_LIBRARY} )

  # tools used for profiling post-processing
  find_program(CF3_PPROF_COMMAND NAMES pprof google-pprof)
  find_program(CF3_DOT_COMMAND dot)

  mark_as_advanced( CF3_PPROF_COMMAND CF3_DOT_COMMAND )

coolfluid_set_package( PACKAGE GooglePerftools
                       DESCRIPTION "memory and performance analysis tools"
                       URL "http://code.google.com/p/google-perftools"
                       TYPE OPTIONAL
                       VARS
                       GOOGLEPERFTOOLS_INCLUDE_DIRS
                       GOOGLEPERFTOOLS_LIBRARIES
                       GOOGLEPERFTOOLS_PROFILER_LIBRARY
                       GOOGLEPERFTOOLS_TCMALLOC_LIBRARY
                       QUIET )
