# this module looks for CGNS library
# it will define the following values
#
# Needs environmental variables
#   CGNS_HOME
# Sets
#   CGNS_INCLUDE_DIR
#   CGNS_LIBRARY
#   CF_HAVE_CGNS
#

SET_TRIAL_INCLUDE_PATH ("") # clear include search path
SET_TRIAL_LIBRARY_PATH ("") # clear library search path

ADD_TRIAL_INCLUDE_PATH( ${CGNS_HOME}/include )
ADD_TRIAL_INCLUDE_PATH( $ENV{CGNS_HOME}/include )

FIND_PATH( CGNS_INCLUDE_DIR cgnslib.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
FIND_PATH( CGNS_INCLUDE_DIR cgnslib.h )

ADD_TRIAL_LIBRARY_PATH(${CGNS_HOME}/lib )
ADD_TRIAL_LIBRARY_PATH($ENV{CGNS_HOME}/lib)

FIND_LIBRARY(CGNS_LIBRARY cgns  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
FIND_LIBRARY(CGNS_LIBRARY cgns )

IF(CGNS_INCLUDE_DIR AND CGNS_LIBRARY)
  SET(CF_HAVE_CGNS 1 CACHE BOOL "Found CGNS library")
ELSE()
  SET(CF_HAVE_CGNS 0 CACHE BOOL "Not fount CGNS library")
ENDIF()

MARK_AS_ADVANCED (
  CGNS_INCLUDE_DIR
  CGNS_LIBRARY
  CF_HAVE_CGNS
)

coolfluid_log( "CF_HAVE_CGNS: [${CF_HAVE_CGNS}]" )
IF(CF_HAVE_CGNS)
   coolfluid_log( "  CGNS_INCLUDE_DIR: [${CGNS_INCLUDE_DIR}]" )
   coolfluid_log( "  CGNS_LIBRARY:     [${CGNS_LIBRARY}]" )
ELSE()
   coolfluid_log_file ( "  CGNS_INCLUDE_DIR: [${CGNS_INCLUDE_DIR}]" )
   coolfluid_log_file ( "  CGNS_LIBRARY:     [${CGNS_LIBRARY}]" )
ENDIF()
