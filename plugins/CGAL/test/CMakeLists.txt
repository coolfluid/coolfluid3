################################################################################
# test tetra sf using cgal

list( APPEND utest-tetra-sf_cflibs coolfluid_mesh_sf coolfluid_cgal coolfluid_mesh_gmsh )
list( APPEND utest-tetra-sf_files  utest-tetra-sf.cpp )

set( utest-tetra-sf_condition  ${coolfluid_cgal_builds} )

coolfluid_add_unittest( utest-tetra-sf )

if( DEFINED coolfluid_cgal_COMPILE_FLAGS )

  set_source_files_properties(utest-tetra-sf.cpp PROPERTIES COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS}")

  if(CF_STRIP_CGAL_COMPILE_FLAGS)
    set(utest-tetra-sf_COMPILE_FLAGS "${utest-tetra-sf_COMPILE_FLAGS} ${coolfluid_cgal_COMPILE_FLAGS}")
  endif()

endif()

################################################################################
