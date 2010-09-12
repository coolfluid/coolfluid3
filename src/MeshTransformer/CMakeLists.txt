coolfluid_find_orphan_files()

list( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

list( APPEND coolfluid-mesh-transformer_libs SF Neu Gmsh)
if(CF_HAVE_CGNS)
  list( APPEND coolfluid-mesh-transformer_libs CGNS3)
endif()
if(CF_BUILD_OpenFOAM)
  list( APPEND coolfluid-mesh-transformer_libs OpenFOAM)
endif()

coolfluid_add_application (coolfluid-mesh-transformer)
