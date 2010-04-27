################################################################################

LIST ( APPEND uTestMesh_libs P1 )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp )

CF_ADD_UNITTEST ( uTestMesh )

LIST ( APPEND uTestLagrangeSF_libs Common P1 )
LIST ( APPEND uTestLagrangeSF_files
  uTestLagrangeSFTriagP1.cpp )

CF_ADD_UNITTEST ( uTestLagrangeSF )

################################################################################
