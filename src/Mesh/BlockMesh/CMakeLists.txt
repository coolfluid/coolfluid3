list( APPEND coolfluid_mesh_block_files
  BlockData.cpp
  BlockData.hpp
  LibBlockMesh.cpp
  LibBlockMesh.hpp
  WriteDict.cpp
  WriteDict.hpp
)

list( APPEND coolfluid_mesh_block_cflibs coolfluid_mesh coolfluid_mesh_sf )

set( coolfluid_mesh_block_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_block )
