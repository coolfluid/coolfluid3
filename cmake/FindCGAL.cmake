#
# this module look for CGAL (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# CGAL_INCLUDE_DIR  = where parmetis.h can be found
# CGAL_LIBRARY      = the library to link against (parmetis etc)
# CF_HAVE_CGAL        = set to true after finding the library
#

option( CF_SKIP_CGAL "Skip search for CGAL library" OFF )

if( NOT CF_SKIP_CGAL )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${CGAL_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{CGAL_ROOT}/include )

  find_path(CGAL_INCLUDE_DIR CGAL ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(CGAL_INCLUDE_DIR CGAL )

  coolfluid_coolfluid_add_trial_library_path(${CGAL_ROOT}/lib )
  coolfluid_coolfluid_add_trial_library_path($ENV{CGAL_ROOT}/lib )

  find_library(CGAL_LIBRARY CGAL ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CGAL_LIBRARY CGAL )

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
