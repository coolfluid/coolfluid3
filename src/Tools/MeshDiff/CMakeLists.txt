coolfluid_find_orphan_files()

list( APPEND coolfluid_meshdiff_files
  LibMeshDiff.cpp
  LibMeshDiff.hpp
  MeshDiff.cpp
  MeshDiff.hpp
)

list( APPEND coolfluid_meshdiff_cflibs coolfluid_mesh)
if( CF_BUILD_GooglePerfTools AND CF_ENABLE_PROFILING )
  list( APPEND coolfluid_meshdiff_cflibs coolfluid_google_perftools )
endif()

coolfluid_add_library( coolfluid_meshdiff )


