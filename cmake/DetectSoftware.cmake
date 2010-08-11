##############################################################################
# finding MPI (essential) must be after the GlobalOptions

LOG ( "\n +++ Searching for MPI ..." )
include ( DetectMPI )

##############################################################################
# finding boost (essential)

LOG ( "\n +++ Searching for Boost ..." )
include ( DetectBoost )

##############################################################################
# Find Qt - defines QT_USE_FILE and QT_LIBRARIES used below
find_package( Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml QtNetwork QtTest )

##############################################################################
# find non essential packages

# using our find macros

LOG ( "" )
find_package(BlasLapack)      # search for Blas Lapack support
find_package(Metis)           # serial domain decomposition
find_package(Parmetis)        # parallel domain decomposition
find_package(Curl)            # curl downloads files on the fly
find_package(Valgrind)        # valgrind for profiling and memmory leak detection
find_package(GooglePerftools) # dynamic profiler and memory checker
find_package(CGNS)            # CGNS library
find_package(CGAL)            # CGAL library

# using cmake find macros

find_package(ZLIB)          # file compression support
LOG ( "ZLIB_FOUND: [${ZLIB_FOUND}]" )
IF ( ZLIB_FOUND )
LOG ( "  ZLIB_INCLUDE_DIRS: [${ZLIB_INCLUDE_DIRS}]" )
LOG ( "  ZLIB_LIBRARIES:    [${ZLIB_LIBRARIES}]" )
ENDIF()

find_package (BZip2)        # file compression support
LOG ( "BZIP2_FOUND: [${BZIP2_FOUND}]" )
IF ( BZIP2_FOUND )
LOG ( "  BZIP2_INCLUDE_DIR:  [${BZIP2_INCLUDE_DIR}]" )
LOG ( "  BZIP2_LIBRARIES:    [${BZIP2_LIBRARIES}]" )
LOG ( "  BZIP2_DEFINITIONS:  [${BZIP2_DEFINITIONS}]" )
LOG ( "  BZIP2_NEED_PREFIX:  [${BZIP2_NEED_PREFIX}]" )
ENDIF()

include ( CheckSvnVersion ) # check for subversion support
