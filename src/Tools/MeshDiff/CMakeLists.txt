coolfluid_find_orphan_files()

list( APPEND MeshDiff_files
  MeshDiff.cpp
  MeshDiff.hpp
)

list( APPEND MeshDiff_cflibs Mesh)
if( CF_BUILD_GooglePerfTools AND CF_ENABLE_PROFILING )
  list( APPEND MeshDiff_cflibs GooglePerfTools )
endif()

coolfluid_add_library( MeshDiff )


