list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_generation
                        KERNEL 
                        SOURCES ${coolfluid_mesh_generation_files}
                        LIBS    coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh )

