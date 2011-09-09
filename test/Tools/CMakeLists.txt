list( APPEND utest-mesh-generation_cflibs coolfluid_mesh_generation coolfluid_mesh_lagrangep1 coolfluid_mesh )
list( APPEND utest-mesh-generation_files
  utest-mesh-generation.cpp )

list( APPEND utest-mesh-generation_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-mesh-generation_condition ${coolfluid_mesh_generation_builds} )

coolfluid_add_unit_test( utest-mesh-generation )



list( APPEND utest-tools-growl_cflibs coolfluid_tools_growl )
list( APPEND utest-tools-growl_files
  utest-tools-growl.cpp
)

coolfluid_add_unit_test( utest-tools-growl )
