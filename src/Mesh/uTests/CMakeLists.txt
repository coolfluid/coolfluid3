################################################################################

LIST ( APPEND uTestMesh_libs Mesh )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp )

CF_ADD_UNITTEST ( uTestMesh )

################################################################################
