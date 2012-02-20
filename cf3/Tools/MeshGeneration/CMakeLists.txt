list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND coolfluid_mesh_generation_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh )

coolfluid_add_library( coolfluid_mesh_generation )

