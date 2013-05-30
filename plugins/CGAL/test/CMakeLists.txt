################################################################################
# test tetra sf using cgal

coolfluid_add_test( UTEST       utest-cgal-tetra-sf
                    CPP         utest-cgal-tetra-sf.cpp
                    LIBS        coolfluid_mesh_lagrangep1 coolfluid_cgal coolfluid_mesh_gmsh ${CGAL_LIBRARY}
                    CONDITION   coolfluid_cgal_builds )
set_target_properties(utest-cgal-tetra-sf PROPERTIES COMPILE_FLAGS -Wno-c++11-extensions)

coolfluid_add_test( UTEST       utest-cgal-tetrahedralize
                    PYTHON      utest-cgal-tetrahedralize.py
                    CONDITION   coolfluid_cgal_builds )

if( DEFINED coolfluid_cgal_COMPILE_FLAGS )

  set_source_files_properties(utest-tetra-sf.cpp PROPERTIES COMPILE_FLAGS "${coolfluid_cgal_COMPILE_FLAGS}")

  if(CF3_STRIP_CGAL_COMPILE_FLAGS)
    set(utest-tetra-sf_COMPILE_FLAGS "${utest-tetra-sf_COMPILE_FLAGS} ${coolfluid_cgal_COMPILE_FLAGS}")
  endif()

endif()

################################################################################
