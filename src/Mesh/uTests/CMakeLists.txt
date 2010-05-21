################################################################################

LIST ( APPEND uTestMesh_cflibs P1 Neu Gmsh )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp 
  TestMeshReading.cpp
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu
    ${CMAKE_CURRENT_BINARY_DIR}/quadtriag.neu COPYONLY)

IF ( CF_HAVE_CGNS )
  LIST ( APPEND uTestMesh_cflibs CGNS3 )
  LIST ( APPEND uTestMesh_files  TestCGNS.cpp )
ELSE()
  CF_MARK_NOT_ORPHAN ( TestCGNS.cpp )
ENDIF()

CF_ADD_UNITTEST ( uTestMesh )

################################################################################

LIST ( APPEND uTestLagrangeSFTriagP1_libs Common P1 )
LIST ( APPEND uTestLagrangeSFTriagP1_files
  uTestLagrangeSFTriagP1.cpp )

# CF_ADD_UNITTEST ( uTestLagrangeSFTriagP1 )

################################################################################

LIST ( APPEND uTestLagrangeSFQuadP1_libs Common P1 )
LIST ( APPEND uTestLagrangeSFQuadP1_files
  uTestLagrangeSFQuadP1.cpp )

# CF_ADD_UNITTEST ( uTestLagrangeSFQuadP1 )

################################################################################

CF_MARK_NOT_ORPHAN ( uTestLagrangeSFTriagP1 uTestLagrangeSFQuadP1.cpp )
