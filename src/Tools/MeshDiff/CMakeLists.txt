list( APPEND coolfluid_meshdiff_files
  LibMeshDiff.cpp
  LibMeshDiff.hpp
  MeshDiff.cpp
  MeshDiff.hpp
)

list( APPEND coolfluid_meshdiff_libs ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

list( APPEND coolfluid_meshdiff_cflibs coolfluid_mesh)

if( CF_ENABLE_PROFILING AND coolfluid_google_perftools_builds )
  list( APPEND coolfluid_meshdiff_cflibs coolfluid_google_perftools )
endif()

coolfluid_add_library( coolfluid_meshdiff )


