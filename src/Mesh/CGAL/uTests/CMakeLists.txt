################################################################################

LIST( APPEND utest-tetra-sf_cflibs coolfluid_sf coolfluid_cgal coolfluid_gmsh )
LIST( APPEND utest-tetra-sf_files
  utest-tetra-sf.cpp )

coolfluid_add_unittest( utest-tetra-sf )

if( DEFINED ICGAL_COMPILE_FLAGS )

if( CMAKE_COMPILER_IS_GNUCC )

  set_source_files_properties(utest-tetra-sf.cpp PROPERTIES COMPILE_FLAGS ${ICGAL_COMPILE_FLAGS})

  if(CF_STRIP_CGAL_COMPILE_FLAGS)
    set(utest-tetra-sf_COMPILE_FLAGS "${utest-tetra-sf_COMPILE_FLAGS} ${ICGAL_COMPILE_FLAGS}")
  endif()
endif( CMAKE_COMPILER_IS_GNUCC )

endif()

################################################################################
