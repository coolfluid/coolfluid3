################################################################################

LIST ( APPEND uTestMesh_libs P1 )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp )

CF_ADD_UNITTEST ( uTestMesh )

LIST ( APPEND uTestLagrangeSF_libs Common )
LIST ( APPEND uTestLagrangeSF_files
  uTestLagrangeSF.cpp )

CF_ADD_UNITTEST ( uTestLagrangeSF )

################################################################################
