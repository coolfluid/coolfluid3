list( APPEND coolfluid-mesh-transformer_files 
        coolfluid-mesh-transformer.cpp
        Transformer.hpp
        Transformer.cpp 
    )

list( APPEND coolfluid-mesh-transformer_cflibs  coolfluid_shell coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_mesh_neutral coolfluid_mesh_gmsh )
list( APPEND coolfluid-mesh-transformer_cflibs ${CF_KERNEL_LIBS} )

# add blockmesh reader if built
if(coolfluid_mesh_cgns3_builds)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_mesh_cgns3 )
endif()

# add blockmesh reader if built
if(coolfluid_mesh_block_reader_builds)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_mesh_block_reader )
endif()

coolfluid_add_application( coolfluid-mesh-transformer )
