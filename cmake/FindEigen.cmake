# this module looks for EIGEN library
# it will define the following values
#
# Needs environmental variables
#   EIGEN_HOME
# Sets
#   EIGEN_INCLUDE_DIR
#   EIGEN_LIBRARY
#   CF_HAVE_EIGEN
#

find_path( EIGEN_INCLUDE_DIR Eigen )

if(EIGEN_INCLUDE_DIR)
  set(CF_HAVE_EIGEN 1 CACHE BOOL "Found EIGEN library")
else()
  set(CF_HAVE_EIGEN 0 CACHE BOOL "Not found EIGEN library")
endif()

mark_as_advanced(
  EIGEN_INCLUDE_DIR
  CF_HAVE_EIGEN
)

coolfluid_log( "CF_HAVE_EIGEN: [${CF_HAVE_EIGEN}]" )
if(CF_HAVE_EIGEN)
   coolfluid_log( "  EIGEN_INCLUDE_DIR: [${EIGEN_INCLUDE_DIR}]" )
else()
   coolfluid_log_file( "  EIGEN_INCLUDE_DIR: [${EIGEN_INCLUDE_DIR}]" )
endif()
