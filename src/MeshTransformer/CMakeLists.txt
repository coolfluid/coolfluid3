CF_FIND_ORPHAN_FILES()

LIST ( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

LIST ( APPEND coolfluid-mesh-transformer_libs Mesh Neu Gmsh CGNS3 P1)

CF_ADD_APP (coolfluid-mesh-transformer)
