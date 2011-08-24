list(APPEND coolfluid_mesh_generation_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND coolfluid_mesh_generation_cflibs coolfluid_mesh )

if( CF_ENABLE_PROFILING AND coolfluid_tools_googleperftools_builds )
  list( APPEND coolfluid_mesh_generation_cflibs coolfluid_tools_googleperftools )
endif()

coolfluid_add_library( coolfluid_mesh_generation )

