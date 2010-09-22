coolfluid_find_orphan_files()

list(APPEND MeshGeneration_files
  LibMeshGeneration.cpp
  LibMeshGeneration.hpp
  MeshGeneration.cpp
  MeshGeneration.hpp
)

list( APPEND MeshGeneration_cflibs Mesh )
if( CF_BUILD_GooglePerfTools )
  list( APPEND MeshGeneration_cflibs GooglePerfTools )
endif()

coolfluid_add_library( MeshGeneration )

