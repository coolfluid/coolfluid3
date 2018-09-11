# The library
list( APPEND coolfluid_meshdiff_files
  LibMeshDiff.cpp
  LibMeshDiff.hpp
  MeshDiff.cpp
  MeshDiff.hpp
)

coolfluid3_add_library( TARGET  coolfluid_meshdiff
                        KERNEL
                        SOURCES ${coolfluid_meshdiff_files}
                        LIBS    coolfluid_mesh
                                ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

############################################################################################################
# The application
list( APPEND coolfluid-meshdiff_files
 coolfluid-meshdiff.cpp
 Commands.hpp
 Commands.cpp
)

list( APPEND coolfluid-meshdiff_cflibs coolfluid_mesh coolfluid_meshdiff coolfluid_shell coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_mesh_smurf )

if(CF3_HAVE_CGNS)
  list( APPEND coolfluid-meshdiff_cflibs coolfluid_mesh_cgns )
endif(CF3_HAVE_CGNS)

coolfluid_add_application( coolfluid-meshdiff )
