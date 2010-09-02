CF_FIND_ORPHAN_FILES()

LIST ( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

LIST ( APPEND coolfluid-mesh-transformer_libs SF Neu Gmsh)
if(CF_HAVE_CGNS)
  LIST ( APPEND coolfluid-mesh-transformer_libs CGNS3)
endif()
if(${OpenFOAM_will_compile})
  LIST ( APPEND coolfluid-mesh-transformer_libs OpenFOAM)
endif()

CF_ADD_APP (coolfluid-mesh-transformer)
