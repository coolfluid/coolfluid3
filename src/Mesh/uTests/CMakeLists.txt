################################################################################

list( APPEND utest-mesh_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_sf )
list( APPEND utest-mesh_files
  uTestMesh.cpp
  TestMeshComponents.cpp
  TestMeshConstruction.cpp
  TestMeshReading.cpp
  TestMeshInterpolation.cpp
)
  
# Only add TestCGNS.cpp if the CGNS library is available
if( ${CF_COMPILES_CGNS3} )
  list( APPEND utest-mesh_cflibs coolfluid_cgns3 )
  list( APPEND utest-mesh_files  TestCGNS.cpp )
  list( APPEND utest-mesh_includedirs ${CGNS_INCLUDE_DIR} )
else()
  coolfluid_mark_not_orphan( TestCGNS.cpp )
endif()

# Add the target
coolfluid_add_unittest( utest-mesh )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-mesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-neureader-mpi_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_sf )
list( APPEND utest-neureader-mpi_files
  uTestNeuReaderMPI.cpp
)
  
# Add the target
coolfluid_add_unittest( utest-neureader-mpi )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-neureader-mpi
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-connectivity-data_cflibs coolfluid_neutral coolfluid_mesh_generation coolfluid_sf )
list( APPEND utest-connectivity-data_files
  utest-connectivity-data.cpp )

coolfluid_add_unittest( utest-connectivity-data )

################################################################################

list( APPEND uTestHexa3DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestHexa3DLagrangeP1_files
  uTestHexa3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestHexa3DLagrangeP1 )

################################################################################

list( APPEND uTestTetra3DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestTetra3DLagrangeP1_files
  uTestTetra3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTetra3DLagrangeP1 )

################################################################################

list( APPEND uTestLine1DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestLine1DLagrangeP1_files
  uTestLine1DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine1DLagrangeP1 )

################################################################################

list( APPEND uTestLine2DLagrangeP1_cflibs coolfluid_mesh_generation coolfluid_sf )
list( APPEND uTestLine2DLagrangeP1_files
  uTestLine2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine2DLagrangeP1 )

################################################################################

list( APPEND uTestLine3DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestLine3DLagrangeP1_files
  uTestLine3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestLine3DLagrangeP1 )

################################################################################

list( APPEND uTestTriag2DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestTriag2DLagrangeP1_files
  uTestTriag2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTriag2DLagrangeP1 )

################################################################################

list( APPEND utest-triag2d-lagrange-P2B_cflibs coolfluid_sf )
list( APPEND utest-triag2d-lagrange-P2B_files
  uTestTriag2DLagrangeP2B.cpp )

coolfluid_add_unittest( utest-triag2d-lagrange-P2B )

################################################################################

list( APPEND uTestTriag3DLagrangeP1_cflibs coolfluid_sf )
list( APPEND uTestTriag3DLagrangeP1_files
  uTestTriag3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestTriag3DLagrangeP1 )

################################################################################

list( APPEND uTestQuad2DLagrangeP1_cflibs  coolfluid_sf )
list( APPEND uTestQuad2DLagrangeP1_files
  uTestQuad2DLagrangeP1.cpp )

coolfluid_add_unittest( uTestQuad2DLagrangeP1 )

################################################################################

list( APPEND uTestQuad3DLagrangeP1_cflibs  coolfluid_sf )
list( APPEND uTestQuad3DLagrangeP1_files
  uTestQuad3DLagrangeP1.cpp )

coolfluid_add_unittest( uTestQuad3DLagrangeP1 )

################################################################################

list( APPEND uTestMatrixInterpolation_cflibs  coolfluid_sf )
list( APPEND uTestMatrixInterpolation_files
  uTestMatrixInterpolation.cpp )

coolfluid_add_unittest( uTestMatrixInterpolation )

################################################################################

list( APPEND utest-nodes_cflibs coolfluid_neutral coolfluid_sf )
list( APPEND utest-nodes_files
  uTestNodes.cpp )

coolfluid_add_unittest( utest-nodes )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-nodes
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-field_cflibs coolfluid_neutral coolfluid_sf )
list( APPEND utest-field_files
  uTestFields.cpp )

coolfluid_add_unittest( utest-field )

################################################################################

list( APPEND utest-volume-sf_cflibs coolfluid_sf coolfluid_mesh )
list( APPEND utest-volume-sf_files
  uTestVolumeSF.cpp )

coolfluid_add_unittest( utest-volume-sf )

################################################################################

list( APPEND utest-vector-benchmark_cflibs coolfluid_sf coolfluid_mesh_generation coolfluid_blockmesh )
list( APPEND utest-vector-benchmark_files
  utest-vector-benchmark.cpp )
if(CF_HAVE_EIGEN)
    list( APPEND utest-vector-benchmark_includedirs ${EIGEN_INCLUDE_DIR} )
endif()
  set( utest-vector-benchmark_profile ON )
coolfluid_add_unittest( utest-vector-benchmark )

################################################################################
