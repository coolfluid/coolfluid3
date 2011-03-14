list( APPEND utest-heat-steady_cflibs coolfluid_mesh coolfluid_field_generation)
list( APPEND utest-heat-steady_files
  utest-heat-steady.cpp )

list( APPEND utest-heat-steady_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-steady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-steady )

list( APPEND utest-heat-unsteady_cflibs coolfluid_mesh )
list( APPEND utest-heat-unsteady_files
  utest-heat-unsteady.cpp )

list( APPEND utest-heat-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-unsteady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-unsteady )
