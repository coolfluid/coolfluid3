# this module looks for CGNS library
# it will define the following values
#
# Needs environmental variables
#   CGNS_HOME
# Sets
#   CGNS_INCLUDE_DIRS
#   CGNS_LIBRARIES
#   CF3_HAVE_CGNS
#

option( CF3_SKIP_CGNS "Skip search for CGNS library" OFF )

    coolfluid_set_trial_include_path("") # clear include search path
    coolfluid_set_trial_library_path("") # clear library search path

    coolfluid_add_trial_include_path( ${CGNS_HOME}/include )
    coolfluid_add_trial_include_path( $ENV{CGNS_HOME}/include )

    find_path( CGNS_INCLUDE_DIRS cgnslib.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
    find_path( CGNS_INCLUDE_DIRS cgnslib.h )

    coolfluid_add_trial_library_path(${CGNS_HOME}/lib )
    coolfluid_add_trial_library_path($ENV{CGNS_HOME}/lib)

    find_library(CGNS_LIBRARIES cgns  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
    find_library(CGNS_LIBRARIES cgns )

    find_library(HDF5_LIBRARIES hdf5  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
    find_library(HDF5_LIBRARIES hdf5 )

    if( HDF5_LIBRARIES )
        set( CGNS_LIBRARIES ${CGNS_LIBRARIES} ${HDF5_LIBRARIES} )
    endif()

coolfluid_set_package( PACKAGE CGNS
                       DESCRIPTION "CFD General Notation System"
                       URL "http://cgns.sourceforge.net"
                       PURPOSE "For CGNS format IO"
                       TYPE OPTIONAL
                       VARS CGNS_INCLUDE_DIRS CGNS_LIBRARIES )
