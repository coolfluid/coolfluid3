# Sets:
# GOOGLE_PERFTOOLS_INCLUDE_DIR  = where google/profiler.h can be found
# GOOGLE_PERFTOOLS_LIBRARIES      = the library to link against
# CF_HAVE_GOOGLE_PERFTOOLS      = set to true after finding the library

option( CF_SKIP_GOOGLE_PERFTOOLS "Skip search for google-perftools" OFF )

if( NOT CF_SKIP_GOOGLE_PERFTOOLS )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${GOOGLE_PERFTOOLS_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{GOOGLE_PERFTOOLS_ROOT}/include )

  find_path(GOOGLE_PERFTOOLS_INCLUDE_DIR google/profiler.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(GOOGLE_PERFTOOLS_INCLUDE_DIR google/profiler.h )

  coolfluid_add_trial_library_path(    ${GOOGLE_PERFTOOLS_ROOT}/lib )
  coolfluid_add_trial_library_path( $ENV{GOOGLE_PERFTOOLS_ROOT}/lib )

  find_library(GOOGLE_PERFTOOLS_PROFILER_LIBRARY profiler ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(GOOGLE_PERFTOOLS_PROFILER_LIBRARY profiler )

  find_library(GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY tcmalloc ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY tcmalloc )

  if( GOOGLE_PERFTOOLS_INCLUDE_DIR AND GOOGLE_PERFTOOLS_PROFILER_LIBRARY AND GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY )
    set( CF_HAVE_GOOGLE_PERFTOOLS 1 CACHE BOOL "Found google-perftools" )
  else()
    set( CF_HAVE_GOOGLE_PERFTOOLS 0 CACHE BOOL "Not fount google-perftools" )
  endif()

  # tools used for profiling post-processing
  find_program(CF_PPROF_COMMAND pprof)
  find_program(CF_DOT_COMMAND dot)


else()

    set(CF_HAVE_GOOGLE_PERFTOOLS 0 CACHE BOOL "Skipped google-perftools")

endif()

set( GOOGLE_PERFTOOLS_LIBRARIES ${GOOGLE_PERFTOOLS_PROFILER_LIBRARY} ${GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY} )

mark_as_advanced( GOOGLE_PERFTOOLS_INCLUDE_DIR
                   GOOGLE_PERFTOOLS_LIBRARIES
                   GOOGLE_PERFTOOLS_PROFILER_LIBRARY
                   GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY
                   CF_HAVE_GOOGLE_PERFTOOLS
                   CF_PPROF_COMMAND
                   CF_DOT_COMMAND )
  
if( ${CF_HAVE_GOOGLE_PERFTOOLS} )
    list( APPEND CF_DEPS_LIBRARIES ${GOOGLE_PERFTOOLS_LIBRARIES} )
endif()

coolfluid_log( "CF_HAVE_GOOGLE_PERFTOOLS: [${CF_HAVE_GOOGLE_PERFTOOLS}]" )
coolfluid_log_file( "  GOOGLE_PERFTOOLS_INCLUDE_DIR:  [${GOOGLE_PERFTOOLS_INCLUDE_DIR}]" )
coolfluid_log_file( "  GOOGLE_PERFTOOLS_PROFILER_LIBRARY: [${GOOGLE_PERFTOOLS_PROFILER_LIBRARY}]" )
coolfluid_log_file( "  GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY: [${GOOGLE_PERFTOOLS_TCMALLOC_LIBRARY}]" )
coolfluid_log_file( "  GOOGLE_PERFTOOLS_LIBRARIES:         [${GOOGLE_PERFTOOLS_LIBRARIES}]" )

