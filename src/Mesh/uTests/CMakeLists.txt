################################################################################

LIST ( APPEND uTestMesh_cflibs P1 Neu Gmsh )
LIST ( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestP1Elements.cpp
  TestMeshConstruction.cpp
  TestMeshReading.cpp
)

IF ( CF_HAVE_CGNS )
  LIST ( APPEND uTestMesh_cflibs CGNS3 )
  LIST ( APPEND uTestMesh_files  TestCGNS.cpp )
  LIST ( APPEND uTestMesh_includedirs ${CGNS_INCLUDE_DIR} )
ELSE()
  CF_MARK_NOT_ORPHAN ( TestCGNS.cpp )
ENDIF()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/quadtriag.neu COPYONLY)

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

LIST ( APPEND uTestIntegration_libs Common P1 Neu )
LIST ( APPEND uTestIntegration_files
  uTestIntegration.cpp )

CF_ADD_PROFILED_UNITTEST ( uTestIntegration )

################################################################################

LIST ( APPEND uTestNodes_libs Common P1 Neu )
LIST ( APPEND uTestNodes_files
  uTestNodes.cpp )

CF_ADD_UNITTEST ( uTestNodes )

################################################################################

