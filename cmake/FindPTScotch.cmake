#
# this module look for PTSCOTCH (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# PTSCOTCH_INCLUDE_DIR  = where ptscotch.h can be found
# PTSCOTCH_LIBRARY      = the library to link against (ptscotch etc)
# CF_HAVE_PTSCOTCH      = set to true after finding the library
#

option( CF_SKIP_PTSCOTCH "Skip search for PTScotch library" OFF )

# dont search for ptscotch without MPI
if( NOT CF_SKIP_PTSCOTCH OR NOT CF_HAVE_MPI )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${PTSCOTCH_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{PTSCOTCH_ROOT}/include )
  
  find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(PTSCOTCH_INCLUDE_DIR ptscotch.h )

  coolfluid_add_trial_library_path(${PTSCOTCH_ROOT}/lib )
  coolfluid_add_trial_library_path($ENV{PTSCOTCH_ROOT}/lib )

  find_library(PTSCOTCH_LIB_PTSCOTCH ptscotch ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCH ptscotch )

  find_library(PTSCOTCH_LIB_SCOTCH scotch ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCH scotch )


  set( PTSCOTCH_LIBRARIES ${PTSCOTCH_LIB_PTSCOTCH} ${PTSCOTCH_LIB_SCOTCH} )

  if(PTSCOTCH_INCLUDE_DIR AND PTSCOTCH_LIBRARIES)
    set(CF_HAVE_PTSCOTCH 1)
  else()
    set(CF_HAVE_PTSCOTCH 0)
  endif()

else()
    set(CF_HAVE_PTSCOTCH 0)
endif()

mark_as_advanced(
  PTSCOTCH_INCLUDE_DIR
  PTSCOTCH_LIB_PTSCOTCH
  PTSCOTCH_LIB_SCOTCH
  PTSCOTCH_LIBRARIES
  CF_HAVE_PTSCOTCH
)

coolfluid_log( "CF_HAVE_PTSCOTCH: [${CF_HAVE_PTSCOTCH}]" )
if(CF_HAVE_PTSCOTCH)
  coolfluid_log( "  PTSCOTCH_INCLUDE_DIR: [${PTSCOTCH_INCLUDE_DIR}]" )
  coolfluid_log( "  PTSCOTCH_LIBRARIES:   [${PTSCOTCH_LIBRARIES}]" )
endif(CF_HAVE_PTSCOTCH)
