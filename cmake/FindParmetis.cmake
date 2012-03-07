#
# this module look for PARMETIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# PARMETIS_INCLUDE_DIRS  = where parmetis.h can be found
# PARMETIS_LIBRARIES     = the library to link against (parmetis etc)
# CF3_HAVE_PARMETIS      = set to true after finding the library
#

option( CF3_SKIP_PARMETIS "Skip search for Parmetis library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${PARMETIS_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{PARMETIS_ROOT}/include )

  find_path(PARMETIS_INCLUDE_DIRS parmetis.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(PARMETIS_INCLUDE_DIRS parmetis.h )

  coolfluid_add_trial_library_path(${PARMETIS_ROOT}/lib )
  coolfluid_add_trial_library_path($ENV{PARMETIS_ROOT}/lib )

  find_library(PARMETIS_LIB_PARMETIS parmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PARMETIS_LIB_PARMETIS parmetis )

  find_library(PARMETIS_LIB_METIS metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PARMETIS_LIB_METIS metis )

  if( PARMETIS_LIB_PARMETIS AND PARMETIS_LIB_METIS )
    set( PARMETIS_LIBRARIES ${PARMETIS_LIB_METIS} ${PARMETIS_LIB_PARMETIS} )
  else()
    set( PARMETIS_LIBRARIES NOT-FOUND )
  endif()

coolfluid_set_package( PACKAGE Parmetis
                       DESCRIPTION "parallel graph partitioning"
                       URL "http://glaros.dtc.umn.edu/gkhome/views/metis"
                       TYPE OPTIONAL
                       VARS
                       PARMETIS_INCLUDE_DIRS
                       PARMETIS_LIBRARIES)
