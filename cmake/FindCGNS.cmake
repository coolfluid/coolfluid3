# this module looks for CGNS library
# it will define the following values
#
# Needs environmental variables
#   CGNS_HOME
# Sets
#   CGNS_INCLUDE_DIR
#   CGNS_LIBRARIES
#   CF_HAVE_CGNS
#

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

coolfluid_add_trial_include_path( ${CGNS_HOME}/include )
coolfluid_add_trial_include_path( $ENV{CGNS_HOME}/include )

find_path( CGNS_INCLUDE_DIR cgnslib.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
find_path( CGNS_INCLUDE_DIR cgnslib.h )

coolfluid_add_trial_library_path(${CGNS_HOME}/lib )
coolfluid_add_trial_library_path($ENV{CGNS_HOME}/lib)

find_library(CGNS_LIBRARIES cgns  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
find_library(CGNS_LIBRARIES cgns )

find_library(HDF5_LIBRARIES hdf5  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
find_library(HDF5_LIBRARIES hdf5 )

if ( HDF5_LIBRARIES )
    set( CGNS_LIBRARIES ${CGNS_LIBRARIES} ${HDF5_LIBRARIES} )
endif()

if(CGNS_INCLUDE_DIR AND CGNS_LIBRARIES)
  set(CF_HAVE_CGNS 1)
else()
  set(CF_HAVE_CGNS 0)
endif()

mark_as_advanced(
  CGNS_INCLUDE_DIR
  CGNS_LIBRARIES
  CF_HAVE_CGNS
)

if ( ${CF_HAVE_CGNS} )
    list( APPEND CF_TP_LIBRARIES ${CGNS_LIBRARIES} )
endif()


coolfluid_log( "CF_HAVE_CGNS: [${CF_HAVE_CGNS}]" )
if(CF_HAVE_CGNS)
   coolfluid_log( "  CGNS_INCLUDE_DIR: [${CGNS_INCLUDE_DIR}]" )
   coolfluid_log( "  CGNS_LIBRARIES:     [${CGNS_LIBRARIES}]" )
else()
   coolfluid_log_file( "  CGNS_INCLUDE_DIR: [${CGNS_INCLUDE_DIR}]" )
   coolfluid_log_file( "  CGNS_LIBRARIES:     [${CGNS_LIBRARIES}]" )
endif()
