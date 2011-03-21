# The library
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

############################################################################################################
# The application
list( APPEND coolfluid-meshdiff_files 
 coolfluid-meshdiff.cpp
 Commands.hpp
 Commands.cpp
)

list( APPEND coolfluid-meshdiff_cflibs coolfluid_mesh coolfluid_meshdiff coolfluid_shell coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_cgns3 coolfluid_mesh_tecplot )

coolfluid_add_application( coolfluid-meshdiff )

# store the path for the acceptance tests
get_property( coolfluid-meshdiff_path TARGET coolfluid-meshdiff PROPERTY LOCATION )
set( coolfluid-meshdiff_path ${coolfluid-meshdiff_path} CACHE "Full path to coolfluid-meshdiff" INTERNAL )
mark_as_advanced( coolfluid-meshdiff_path )
