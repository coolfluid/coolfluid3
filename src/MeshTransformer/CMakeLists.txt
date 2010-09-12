CF_FIND_ORPHAN_FILES()

LIST ( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

LIST ( APPEND coolfluid-mesh-transformer_libs SF Neu Gmsh)
if(CF_HAVE_CGNS)
  LIST ( APPEND coolfluid-mesh-transformer_libs CGNS3)
endif()
if(CF_BUILD_OpenFOAM)
  LIST ( APPEND coolfluid-mesh-transformer_libs OpenFOAM)
endif()

coolfluid_add_application (coolfluid-mesh-transformer)
