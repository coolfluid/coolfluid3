list( APPEND utest-heat-steady_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_neutral )
list( APPEND utest-heat-steady_files
  utest-heat-steady.cpp )

list( APPEND utest-heat-steady_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

coolfluid_add_unittest( utest-heat-steady )
