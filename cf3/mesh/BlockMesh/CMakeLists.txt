list( APPEND coolfluid_mesh_blockmesh_files
  BlockData.cpp
  BlockData.hpp
  ChannelGenerator.hpp
  ChannelGenerator.cpp
  LibBlockMesh.cpp
  LibBlockMesh.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_blockmesh
                        KERNEL
                        SOURCES ${coolfluid_mesh_blockmesh_files}
                        LIBS    coolfluid_mesh coolfluid_mesh_lagrangep1 )
