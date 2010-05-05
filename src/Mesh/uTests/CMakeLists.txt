################################################################################

LIST ( APPEND uTestMesh_libs P1 Neu )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp 
  TestMeshReading.cpp )

CF_ADD_UNITTEST ( uTestMesh )

################################################################################

LIST ( APPEND uTestLagrangeSFTriagP1_libs Common P1 )
LIST ( APPEND uTestLagrangeSFTriagP1_files
  uTestLagrangeSFTriagP1.cpp )

CF_ADD_UNITTEST ( uTestLagrangeSFTriagP1 )

################################################################################

LIST ( APPEND uTestLagrangeSFQuadP1_libs Common P1 )
LIST ( APPEND uTestLagrangeSFQuadP1_files
  uTestLagrangeSFQuadP1.cpp )

CF_ADD_UNITTEST ( uTestLagrangeSFQuadP1 )

################################################################################