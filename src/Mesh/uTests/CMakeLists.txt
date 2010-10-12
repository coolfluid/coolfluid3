################################################################################

list( APPEND uTestMesh_cflibs Neu Gmsh SF )
list( APPEND uTestMesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp
  TestMeshReading.cpp
  TestMeshInterpolation.cpp
)
  
# Only add TestCGNS.cpp if the CGNS library is available
if( ${CF_COMPILES_CGNS3} )
  list( APPEND uTestMesh_cflibs CGNS3 )
  list( APPEND uTestMesh_files  TestCGNS.cpp )
  list( APPEND uTestMesh_includedirs ${CGNS_INCLUDE_DIR} )
else()
  coolfluid_mark_not_orphan( TestCGNS.cpp )
endif()

# Add the target
coolfluid_add_unittest( uTestMesh )

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

list( APPEND uTestNeuReaderMPI_cflibs Neu Gmsh SF )
list( APPEND uTestNeuReaderMPI_files
  uTestNeuReaderMPI.cpp
)
  
# Add the target
coolfluid_add_unittest( uTestNeuReaderMPI )

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

list( APPEND uTestConnectivityData_cflibs Neu MeshGeneration SF )
list( APPEND uTestConnectivityData_files
  uTestConnectivityData.cpp )

coolfluid_add_unittest( uTestConnectivityData )

################################################################################

list( APPEND uTestHexa3DLagrangeP1_cflibs SF )
list( APPEND uTestHexa3DLagrangeP1_files
  uTestHexa3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestHexa3DLagrangeP1 )

################################################################################

list( APPEND uTestTetra3DLagrangeP1_cflibs SF )
list( APPEND uTestTetra3DLagrangeP1_files
  uTestTetra3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTetra3DLagrangeP1 )

################################################################################

list( APPEND uTestLine1DLagrangeP1_cflibs SF )
list( APPEND uTestLine1DLagrangeP1_files
  uTestLine1DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine1DLagrangeP1 )

################################################################################

list( APPEND uTestLine2DLagrangeP1_cflibs MeshGeneration SF )
list( APPEND uTestLine2DLagrangeP1_files
  uTestLine2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine2DLagrangeP1 )

################################################################################

list( APPEND uTestLine3DLagrangeP1_cflibs SF )
list( APPEND uTestLine3DLagrangeP1_files
  uTestLine3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine3DLagrangeP1 )

################################################################################

list( APPEND uTestTriag2DLagrangeP1_cflibs SF )
list( APPEND uTestTriag2DLagrangeP1_files
  uTestTriag2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTriag2DLagrangeP1 )

################################################################################

list( APPEND uTestTriag3DLagrangeP1_cflibs SF )
list( APPEND uTestTriag3DLagrangeP1_files
  uTestTriag3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTriag3DLagrangeP1 )

################################################################################

list( APPEND uTestQuad2DLagrangeP1_cflibs  SF )
list( APPEND uTestQuad2DLagrangeP1_files
  uTestQuad2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestQuad2DLagrangeP1 )

################################################################################

list( APPEND uTestQuad3DLagrangeP1_cflibs  SF )
list( APPEND uTestQuad3DLagrangeP1_files
  uTestQuad3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestQuad3DLagrangeP1 )

################################################################################

list( APPEND uTestMatrixInterpolation_cflibs  SF )
list( APPEND uTestMatrixInterpolation_files
  uTestMatrixInterpolation.cpp )

coolfluid_add_unittest( uTestMatrixInterpolation )

################################################################################

list( APPEND uTestIntegration_cflibs Neu Testing MeshGeneration SF )
list( APPEND uTestIntegration_files
  uTestIntegration.cpp )

set( uTestIntegration_profile ON )
coolfluid_add_unittest( uTestIntegration )

################################################################################

list( APPEND uTestNodes_cflibs Neu SF )
list( APPEND uTestNodes_files
  uTestNodes.cpp )

coolfluid_add_unittest( uTestNodes )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET uTestNodes
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND uTestField_cflibs Neu SF)
list( APPEND uTestField_files
  uTestFields.cpp )

coolfluid_add_unittest( uTestField )

################################################################################

list( APPEND uTestVolumeSF_cflibs SF Mesh )
list( APPEND uTestVolumeSF_files
  uTestVolumeSF.cpp )

coolfluid_add_unittest( uTestVolumeSF )

################################################################################

list( APPEND uTestVectorBenchmark_cflibs SF MeshGeneration BlockMesh )
list( APPEND uTestVectorBenchmark_files
  uTestVectorBenchmark.cpp )
if(CF_HAVE_EIGEN)
    list( APPEND uTestVectorBenchmark_includedirs ${EIGEN_INCLUDE_DIR} )
endif()
  set( uTestVectorBenchmark_profile ON )
coolfluid_add_unittest( uTestVectorBenchmark )

################################################################################
