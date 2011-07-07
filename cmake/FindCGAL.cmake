#
# this module look for CGAL (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# CGAL_INCLUDE_DIR  = where parmetis.h can be found
# CGAL_LIBRARIES      = the library to link against (parmetis etc)
# CF_HAVE_CGAL        = set to true after finding the library
#

option( CF_SKIP_CGAL "Skip search for CGAL library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${CGAL_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{CGAL_ROOT}/include )

  find_path(CGAL_INCLUDE_DIR CGAL ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(CGAL_INCLUDE_DIR CGAL )

  coolfluid_add_trial_library_path(${CGAL_ROOT}/lib )
  coolfluid_add_trial_library_path($ENV{CGAL_ROOT}/lib )

  find_library(CGAL_LIBRARIES CGAL ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CGAL_LIBRARIES CGAL )

coolfluid_add_package( PACKAGE CGAL DESCRIPTION "geometric algorithms" URL "http://www.cgal.org"
                       VARS
                       CGAL_INCLUDE_DIR CGAL_LIBRARIES )
