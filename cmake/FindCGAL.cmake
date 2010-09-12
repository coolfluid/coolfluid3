#
# this module look for CGAL (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# CGAL_INCLUDE_DIR  = where parmetis.h can be found
# CGAL_LIBRARY      = the library to link against (parmetis etc)
# CF_HAVE_CGAL        = set to true after finding the library
#

OPTION( CF_SKIP_CGAL "Skip search for CGAL library" OFF )

if( NOT CF_SKIP_CGAL )

  SET_TRIAL_INCLUDE_PATH("") # clear include search path
  SET_TRIAL_LIBRARY_PATH("") # clear library search path

  ADD_TRIAL_INCLUDE_PATH( ${CGAL_ROOT}/include )
  ADD_TRIAL_INCLUDE_PATH( $ENV{CGAL_ROOT}/include )

  FIND_PATH(CGAL_INCLUDE_DIR CGAL ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  FIND_PATH(CGAL_INCLUDE_DIR CGAL )

  ADD_TRIAL_LIBRARY_PATH(${CGAL_ROOT}/lib )
  ADD_TRIAL_LIBRARY_PATH($ENV{CGAL_ROOT}/lib )

  FIND_LIBRARY(CGAL_LIBRARY CGAL ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  FIND_LIBRARY(CGAL_LIBRARY CGAL )

  if(CGAL_INCLUDE_DIR AND CGAL_LIBRARY)
    set(CF_HAVE_CGAL 1)
  endif()

endif()

if( NOT DEFINED CF_HAVE_CGAL )
    set(CF_HAVE_CGAL 0)
endif()

mark_as_advanced(
  CGAL_INCLUDE_DIR
  CGAL_LIBRARY
  CF_HAVE_CGAL
)

coolfluid_log( "CF_HAVE_CGAL: [${CF_HAVE_CGAL}]" )
if(CF_HAVE_CGAL)
  coolfluid_log( "  CGAL_INCLUDE_DIR: [${CGAL_INCLUDE_DIR}]" )
  coolfluid_log( "  CGAL_LIBRARIES:   [${CGAL_LIBRARIES}]" )
endif(CF_HAVE_CGAL)
