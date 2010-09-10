# CF_FIND_ORPHAN_FILES()

###############################################################################
### test for rapidxml
ADD_SUBDIRECTORY( rapidxml )

###############################################################################
### test for xmlnode
ADD_SUBDIRECTORY( xmlnode )

###############################################################################
### test for boost::lambda
ADD_SUBDIRECTORY( boost_lambda )

###############################################################################
### test for boost::bind
ADD_SUBDIRECTORY( boost_bind )

###############################################################################
### test for boost::multi_array
ADD_SUBDIRECTORY( boost_multiarray )

###############################################################################
### test for using boost::progress_display with LogStream
ADD_SUBDIRECTORY( LogWithProgressDisplay )

###############################################################################
### test for CGAL functions
ADD_SUBDIRECTORY( cgal )

###############################################################################
### test for CUDA
ADD_SUBDIRECTORY( cuda )

###############################################################################
### test for OpenCL
ADD_SUBDIRECTORY( opencl )


