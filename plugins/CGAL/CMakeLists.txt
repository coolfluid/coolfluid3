##############################################################################
# CGAL Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( CGAL CXX )

find_package(CGAL)

option(CF_STRIP_CGAL_COMPILE_FLAGS "Strip optimization and compile flags for CGAL, to avoid the long compilation" ON)

# avoid a conflict between boost and CGAL
add_definitions(-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE)


if( CF_HAVE_CGAL )

  include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

  set( coolfluid_cgal_COMPILE_FLAGS "" )
  if( CMAKE_COMPILER_IS_GNUCC )

    set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -frounding-math")

    # just for this library, disable optimizations
    # to avoid enormous compilation times and memory usage
    if(CF_STRIP_CGAL_COMPILE_FLAGS)
      set(coolfluid_cgal_COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS} -g0 -O0")
    endif()

  endif()

  # add dir with CGAL sources
  add_subdirectory( CGAL )

  # add dir with CGAL unit tests
  if( CF_ENABLE_UNITTESTS )
    add_subdirectory( test )
  endif()

else()

  coolfluid_mark_not_orphan(
    test/utest-tetra-sf.cpp
    CGAL/LibCGAL.hpp
    CGAL/LibCGAL.cpp
    CGAL/ImplicitFunctionMesh.cpp
    CGAL/ImplicitFunctionMesh.hpp  )

endif()

