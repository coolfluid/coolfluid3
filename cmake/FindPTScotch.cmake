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

  find_library(PTSCOTCH_LIB_PTSCOTCHERR ptscotcherr ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCHERR ptscotcherr )

  find_library(PTSCOTCH_LIB_PTSCOTCHERREXIT ptscotcherrexit ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCHERREXIT ptscotcherrexit )
  
  #find_library(PTSCOTCH_LIB_PTSCOTCHPARMETIS ptscotchparmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  #find_library(PTSCOTCH_LIB_PTSCOTCHPARMETIS ptscotchparmetis )
  
  find_library(PTSCOTCH_LIB_SCOTCH scotch ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCH scotch )

  find_library(PTSCOTCH_LIB_SCOTCHERR scotcherr ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCHERR scotcherr )
                                   
  find_library(PTSCOTCH_LIB_SCOTCHERREXIT scotcherrexit ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCHERREXIT scotcherrexit )
                                   
  #find_library(PTSCOTCH_LIB_SCOTCHMETIS scotchmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  #find_library(PTSCOTCH_LIB_SCOTCHMETIS scotchmetis )

  set( PTSCOTCH_LIBRARIES 
        ${PTSCOTCH_LIB_PTSCOTCH} 
        ${PTSCOTCH_LIB_PTSCOTCHERR} 
        ${PTSCOTCH_LIB_PTSCOTCHERREXIT} 
        #${PTSCOTCH_LIB_PTSCOTCHPARMETIS}
        ${PTSCOTCH_LIB_SCOTCH}   
        ${PTSCOTCH_LIB_SCOTCHERR}   
        ${PTSCOTCH_LIB_SCOTCHERREXIT}   
        #${PTSCOTCH_LIB_SCOTCHMETIS}
      )

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
