list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND coolfluid_mesh_generation_cflibs coolfluid_mesh )

coolfluid_add_library( coolfluid_mesh_generation )

