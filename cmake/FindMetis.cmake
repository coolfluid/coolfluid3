#
# this module look for METIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# METIS_INCLUDE_DIRS  = where metis.h can be found
# METIS_LIBRARIES     = the library to link against (metis etc)
# CF3_HAVE_METIS      = set to true after finding the library
#

option( CF3_SKIP_METIS "Skip search for Metis library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${METIS_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{METIS_HOME}/include )

  find_path(METIS_INCLUDE_DIRS metis.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(METIS_INCLUDE_DIRS metis.h)

  coolfluid_add_trial_library_path(${METIS_HOME}/lib )
  coolfluid_add_trial_library_path($ENV{METIS_HOME}/lib )

  find_library(METIS_LIBRARIES metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(METIS_LIBRARIES metis )

coolfluid_set_package( PACKAGE METIS
                       DESCRIPTION "Serial graph partitioning"
                       URL "http://glaros.dtc.umn.edu/gkhome/views/metis"
                       TYPE OPTIONAL
                       VARS METIS_INCLUDE_DIRS METIS_LIBRARIES )
