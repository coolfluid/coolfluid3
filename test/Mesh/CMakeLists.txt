################################################################################

list( APPEND utest-mesh-hash_cflibs coolfluid_mesh )
list( APPEND utest-mesh-hash_files  utest-mesh-hash.cpp )

coolfluid_add_unittest( utest-mesh-hash )

################################################################################

list( APPEND utest-mesh-interpolation_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-interpolation_files  utest-mesh-interpolation.cpp )

coolfluid_add_unittest( utest-mesh-interpolation )

################################################################################

list( APPEND utest-mesh-construction_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-construction_files  utest-mesh-construction.cpp )

coolfluid_add_unittest( utest-mesh-construction )

################################################################################

list( APPEND utest-mesh-reading_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-reading_files  utest-mesh-reading.cpp )

coolfluid_add_unittest( utest-mesh-reading )

################################################################################

list( APPEND utest-mesh-components_cflibs coolfluid_mesh)
list( APPEND utest-mesh-components_files  utest-mesh-components.cpp  )

coolfluid_add_unittest( utest-mesh-components )

################################################################################

# Only add TestCGNS.cpp if the CGNS library is available
if( ${CF_COMPILES_CGNS3} )

  list( APPEND utest-cgns_cflibs coolfluid_cgns3 )
  list( APPEND utest-cgns_files  utest-cgns.cpp )
  list( APPEND utest-cgns_includedirs ${CGNS_INCLUDE_DIR} )

  coolfluid_add_unittest( utest-cgns )

else()
  coolfluid_mark_not_orphan( utest-cgns.cpp )
endif()

# Add the target

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-mesh-components
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-neu-reader-mpi_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_mesh_sf )
list( APPEND utest-neu-reader-mpi_files  utest-neu-reader-mpi.cpp )
  
# Add the target
coolfluid_add_mpi_unittest( utest-neu-reader-mpi )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-neu-reader-mpi
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-gmsh-reader-mpi_cflibs coolfluid_gmsh coolfluid_mesh_sf )
list( APPEND utest-gmsh-reader-mpi_files  utest-gmsh-reader-mpi.cpp )

# Add the target
coolfluid_add_unittest( utest-gmsh-reader-mpi )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
#add_custom_command(TARGET utest-neu-reader-mpi
#                   POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu #${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
#                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    #${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}





################################################################################

list( APPEND utest-connectivity-data_cflibs coolfluid_neutral coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-connectivity-data_files  utest-connectivity-data.cpp )

coolfluid_add_unittest( utest-connectivity-data )

################################################################################

list( APPEND utest-hexa3d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-hexa3d-lagrange-p1_files utest-hexa3d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-hexa3d-lagrange-p1 )

################################################################################

list( APPEND utest-tetra3d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-tetra3d-lagrange-p1_files  utest-tetra3d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-tetra3d-lagrange-p1 )

################################################################################

list( APPEND utest-line1d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-line1d-lagrange-p1_files  utest-line1d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-line1d-lagrange-p1 )

################################################################################

list( APPEND utest-line2d-lagrange-p1_cflibs coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-line2d-lagrange-p1_files  utest-line2d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-line2d-lagrange-p1 )

################################################################################

list( APPEND utest-line3d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-line3d-lagrange-p1_files  utest-line3d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-line3d-lagrange-p1 )

################################################################################

list( APPEND utest-triag2d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-triag2d-lagrange-p1_files  utest-triag2d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-triag2d-lagrange-p1 )

################################################################################

list( APPEND utest-triag2d-lagrange-p2b_cflibs coolfluid_mesh_sf )
list( APPEND utest-triag2d-lagrange-p2b_files utest-triag2d-lagrange-p2b.cpp )

coolfluid_add_unittest( utest-triag2d-lagrange-p2b )

################################################################################

list( APPEND utest-triag3d-lagrange-p1_cflibs coolfluid_mesh_sf )
list( APPEND utest-triag3d-lagrange-p1_files  utest-triag3d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-triag3d-lagrange-p1 )

################################################################################

list( APPEND utest-quad2d-lagrange-p1_cflibs  coolfluid_mesh_sf )
list( APPEND utest-quad2d-lagrange-p1_files   utest-quad2d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-quad2d-lagrange-p1 )

################################################################################

list( APPEND utest-quad3d-lagrange-p1_cflibs  coolfluid_mesh_sf )
list( APPEND utest-quad3d-lagrange-p1_files   utest-quad3d-lagrange-p1.cpp )

coolfluid_add_unittest( utest-quad3d-lagrange-p1 )

################################################################################

list( APPEND utest-matrix-interpolation_cflibs  coolfluid_mesh_sf )
list( APPEND utest-matrix-interpolation_files   utest-matrix-interpolation.cpp )

coolfluid_add_unittest( utest-matrix-interpolation )

################################################################################

list( APPEND utest-nodes_cflibs coolfluid_neutral coolfluid_mesh_sf )
list( APPEND utest-nodes_files  utest-nodes.cpp )

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

list( APPEND utest-field_cflibs coolfluid_neutral coolfluid_mesh_sf coolfluid_gmsh )
list( APPEND utest-field_files  utest-fields.cpp )

coolfluid_add_unittest( utest-field )

################################################################################

list( APPEND utest-volume-sf_cflibs coolfluid_mesh_sf coolfluid_mesh )
list( APPEND utest-volume-sf_files  utest-volume-sf.cpp )

coolfluid_add_unittest( utest-volume-sf )

################################################################################

list( APPEND utest-vector-benchmark_cflibs coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_blockmesh )
list( APPEND utest-vector-benchmark_files  utest-vector-benchmark.cpp )

set( utest-vector-benchmark_profile ON )

coolfluid_add_unittest( utest-vector-benchmark )

################################################################################

if( ${CF_HAVE_PTSCOTCH} )
  list( APPEND utest-pt-scotch_cflibs coolfluid_neutral coolfluid_mesh_sf coolfluid_gmsh )
  list( APPEND utest-pt-scotch_libs ${PTSCOTCH_LIBRARIES} )

  list( APPEND utest-pt-scotch_files  utest-pt-scotch.cpp )
  list( APPEND utest-pt-scotch_includedirs ${PTSCOTCH_INCLUDE_DIR} )

  coolfluid_add_unittest( utest-pt-scotch )

  # Custom post-build rule (must be defined after adding target):
  # Copy the file quadtriag.neu from source dir to binary dir.
  # this must happen after building, as some IDE like Xcode define their
  # build types internally, in which the binary dir exists
  add_custom_command(TARGET utest-pt-scotch
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
else()
  coolfluid_mark_not_orphan( utest-pt-scotch.cpp )
endif()

################################################################################

if( ${CF_HAVE_ZOLTAN} )
  list( APPEND utest-zoltan_cflibs coolfluid_neutral coolfluid_gmsh coolfluid_zoltan )
  list( APPEND utest-zoltan_libs ${ZOLTAN_LIBRARIES} )
  list( APPEND utest-zoltan_files  utest-zoltan.cpp )
  list( APPEND utest-zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} )

  coolfluid_add_mpi_unittest( utest-zoltan )

  # Custom post-build rule (must be defined after adding target):
  # Copy the file quadtriag.neu from source dir to binary dir.
  # this must happen after building, as some IDE like Xcode define their
  # build types internally, in which the binary dir exists
  add_custom_command(TARGET utest-zoltan
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
else()
  coolfluid_mark_not_orphan( utest-zoltan.cpp )
endif()

################################################################################

list( APPEND utest-blockmesh-mpi_cflibs coolfluid_blockmesh coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-blockmesh-mpi_files
  uTestBlockMeshMPI.cpp )
set( utest-blockmesh-mpi_args ${CF_MPI_TESTS_SIZE} )
set( utest-blockmesh-mpi_scaling ON )
coolfluid_add_mpi_unittest( utest-blockmesh-mpi )

################################################################################

list( APPEND utest-blockmesh-mpi-scale_cflibs coolfluid_blockmesh coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-blockmesh-mpi-scale_files
  uTestBlockMeshMPI.cpp )
set( utest-blockmesh-mpi-scale_args "${CF_MPI_TESTS_SIZE} scale" )
set( utest-blockmesh-mpi-scale_scaling ON )
coolfluid_add_mpi_unittest( utest-blockmesh-mpi-scale )



