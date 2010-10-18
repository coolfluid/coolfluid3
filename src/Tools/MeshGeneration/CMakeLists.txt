coolfluid_find_orphan_files()

list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND coolfluid_mesh_generation_cflibs coolfluid_mesh )
if( CF_BUILD_GooglePerfTools )
  list( APPEND coolfluid_mesh_generation_cflibs coolfluid_google_perftools )
endif()

coolfluid_add_library( coolfluid_mesh_generation )

