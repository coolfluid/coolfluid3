list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND coolfluid_mesh_generation_cflibs coolfluid_mesh )

if( CF_ENABLE_PROFILING AND coolfluid_google_perftools_builds )
  list( APPEND coolfluid_mesh_generation_cflibs coolfluid_google_perftools )
endif()

coolfluid_add_library( coolfluid_mesh_generation )

