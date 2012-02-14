list( APPEND coolfluid_mesh_blockmesh_files
  BlockData.cpp
  BlockData.hpp
  ChannelGenerator.hpp
  ChannelGenerator.cpp
  LibBlockMesh.cpp
  LibBlockMesh.hpp
)

list( APPEND coolfluid_mesh_blockmesh_cflibs coolfluid_mesh coolfluid_mesh_lagrangep1 )

set( coolfluid_mesh_blockmesh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_blockmesh )
