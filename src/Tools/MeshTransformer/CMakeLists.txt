coolfluid_find_orphan_files()

list( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

list( APPEND coolfluid-mesh-transformer_libs  coolfluid_mesh_sf coolfluid_mesh_neutral coolfluid_mesh_gmsh )

# add blockmesh reader if built
if(CF_BUILD_coolfluid_mesh_cgns3)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_mesh_cgns3 )
endif()

# add blockmesh reader if built
if(CF_BUILD_coolfluid_mesh_block_reader)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_mesh_block_reader )
endif()

coolfluid_add_application( coolfluid-mesh-transformer )
