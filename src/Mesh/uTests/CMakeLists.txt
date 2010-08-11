################################################################################

LIST( APPEND uTestMesh_cflibs Neu Gmsh SF )
LIST( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp
  TestMeshReading.cpp
  TestMeshInterpolation.cpp
)
  
# Only add TestCGNS.cpp if the CGNS library is available
IF( CF_CGNS3_will_compile )
  LIST( APPEND uTestMesh_cflibs CGNS3 )
  LIST( APPEND uTestMesh_files  TestCGNS.cpp )
  LIST( APPEND uTestMesh_includedirs ${CGNS_INCLUDE_DIR} )
ELSE()
  CF_MARK_NOT_ORPHAN( TestCGNS.cpp )
ENDIF()

# Add the target
CF_ADD_UNITTEST( uTestMesh )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET uTestMesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

LIST( APPEND uTestHexa3DLagrangeP1_cflibs SF )
LIST( APPEND uTestHexa3DLagrangeP1_files
  uTestHexa3DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestHexa3DLagrangeP1 )

################################################################################

LIST( APPEND uTestTetra3DLagrangeP1_cflibs SF )
LIST( APPEND uTestTetra3DLagrangeP1_files
  uTestTetra3DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestTetra3DLagrangeP1 )

################################################################################

LIST( APPEND uTestLine1DLagrangeP1_cflibs SF )
LIST( APPEND uTestLine1DLagrangeP1_files
  uTestLine1DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestLine1DLagrangeP1 )

################################################################################

LIST( APPEND uTestLine2DLagrangeP1_cflibs SF )
LIST( APPEND uTestLine2DLagrangeP1_files
  uTestLine2DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestLine2DLagrangeP1 )

################################################################################

LIST( APPEND uTestLine3DLagrangeP1_cflibs SF )
LIST( APPEND uTestLine3DLagrangeP1_files
  uTestLine3DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestLine3DLagrangeP1 )

################################################################################

LIST( APPEND uTestTriag2DLagrangeP1_cflibs SF )
LIST( APPEND uTestTriag2DLagrangeP1_files
  uTestTriag2DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestTriag2DLagrangeP1 )

################################################################################

LIST( APPEND uTestTriag3DLagrangeP1_cflibs SF )
LIST( APPEND uTestTriag3DLagrangeP1_files
  uTestTriag3DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestTriag3DLagrangeP1 )

################################################################################

LIST( APPEND uTestQuad2DLagrangeP1_cflibs  SF )
LIST( APPEND uTestQuad2DLagrangeP1_files
  uTestQuad2DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestQuad2DLagrangeP1 )

################################################################################

LIST( APPEND uTestQuad3DLagrangeP1_cflibs  SF )
LIST( APPEND uTestQuad3DLagrangeP1_files
  uTestQuad3DLagrangeP1.cpp )

CF_ADD_UNITTEST( uTestQuad3DLagrangeP1 )

################################################################################

LIST( APPEND uTestIntegration_cflibs Neu Testing SF )
LIST( APPEND uTestIntegration_files
  uTestIntegration.cpp )

SET( uTestIntegration_profile ON )
CF_ADD_UNITTEST( uTestIntegration )

################################################################################

LIST( APPEND uTestNodes_cflibs Neu SF )
LIST( APPEND uTestNodes_files
  uTestNodes.cpp )

CF_ADD_UNITTEST( uTestNodes )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET uTestNodes
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

