#
# this module look for PTSCOTCH (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# PTSCOTCH_INCLUDE_DIRS = where ptscotch.h can be found
# PTSCOTCH_LIBRARIES    = the library to link against (ptscotch etc)
# CF3_HAVE_PTSCOTCH     = set to true after finding the library
#

option( CF3_SKIP_PTSCOTCH "Skip search for PTScotch library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${PTSCOTCH_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{PTSCOTCH_ROOT}/include )

  find_path(PTSCOTCH_INCLUDE_DIRS ptscotch.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(PTSCOTCH_INCLUDE_DIRS ptscotch.h )

  coolfluid_add_trial_library_path(${PTSCOTCH_ROOT}/lib )
  coolfluid_add_trial_library_path($ENV{PTSCOTCH_ROOT}/lib )

  find_library(PTSCOTCH_LIB_PTSCOTCH ptscotch ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCH ptscotch )

  find_library(PTSCOTCH_LIB_PTSCOTCHERR ptscotcherr ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCHERR ptscotcherr )

  find_library(PTSCOTCH_LIB_PTSCOTCHERREXIT ptscotcherrexit ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCHERREXIT ptscotcherrexit )

  find_library(PTSCOTCH_LIB_PTSCOTCHPARMETIS ptscotchparmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_PTSCOTCHPARMETIS ptscotchparmetis )

  find_library(PTSCOTCH_LIB_SCOTCH scotch ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCH scotch )

  find_library(PTSCOTCH_LIB_SCOTCHERR scotcherr ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCHERR scotcherr )

  find_library(PTSCOTCH_LIB_SCOTCHERREXIT scotcherrexit ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCHERREXIT scotcherrexit )

  find_library(PTSCOTCH_LIB_SCOTCHMETIS scotchmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PTSCOTCH_LIB_SCOTCHMETIS scotchmetis )

  mark_as_advanced(
  PTSCOTCH_LIB_SCOTCH
  PTSCOTCH_LIB_SCOTCHERR
  PTSCOTCH_LIB_SCOTCHERREXIT
  PTSCOTCH_LIB_SCOTCHMETIS
  PTSCOTCH_LIB_PTSCOTCH
  PTSCOTCH_LIB_PTSCOTCHERR
  PTSCOTCH_LIB_PTSCOTCHERREXIT
  PTSCOTCH_LIB_PTSCOTCHPARMETIS
  )

  set( PTSCOTCH_LIBRARIES
        #${PTSCOTCH_LIB_SCOTCH}
        #${PTSCOTCH_LIB_SCOTCHERR}
        #${PTSCOTCH_LIB_SCOTCHERREXIT}
        #${PTSCOTCH_LIB_SCOTCHMETIS}
        ${PTSCOTCH_LIB_PTSCOTCH}
        ${PTSCOTCH_LIB_PTSCOTCHERR}
        #${PTSCOTCH_LIB_PTSCOTCHERREXIT}
        #${PTSCOTCH_LIB_PTSCOTCHPARMETIS}
      )

  if(ZLIB_FOUND)
      list( APPEND PTSCOTCH_EXTRA_LIBRARIES ${ZLIB_LIBRARIES} )
  endif()

coolfluid_set_package( PACKAGE PTScotch
                       DESCRIPTION "parallel graph partitioning"
                       URL "http://www.labri.fr/perso/pelegrin/scotch"
                       TYPE OPTIONAL
                       VARS
                       PTSCOTCH_INCLUDE_DIRS
                       PTSCOTCH_LIBRARIES PTSCOTCH_LIB_SCOTCH PTSCOTCH_LIB_PTSCOTCHERR PTSCOTCH_EXTRA_LIBRARIES )
