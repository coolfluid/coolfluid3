##############################################################################
# CGAL Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( CGAL CXX )

coolfluid_define_plugin( CGAL ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(CGAL QUIET)
if(CGAL_FOUND)
  include(${CGAL_USE_FILE})
endif()
find_package(GMP QUIET)
find_package(MPFR QUIET)

if( CF3_PLUGIN_CGAL AND CGAL_FOUND AND GMP_FOUND AND MPFR_FOUND )
  # disable optimizations to avoid enormous compilation times and memory usage
  option(CF3_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

  # avoid a conflict between boost and CGAL
  add_definitions(-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE)

  coolfluid_find_orphan_files()

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  set( coolfluid_cgal_COMPILE_FLAGS "" )

  if( CMAKE_COMPILER_IS_GNUCC )

    set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -frounding-math")

    if(coolfluid_cgal_COMPILE_FLAGS)
      set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -g0 -O0")
    endif()

  endif()

  # add dir with CGAL sources
  add_subdirectory( CGAL )

  # add dir with CGAL unit tests
  add_subdirectory( test )

  # add dir with CGAL sandbox
  add_subdirectory( sandbox EXCLUDE_FROM_ALL )

endif()
