################################################################################

list( APPEND utest-mesh-hash_cflibs coolfluid_mesh )
list( APPEND utest-mesh-hash_files  utest-mesh-hash.cpp )

coolfluid_add_unittest( utest-mesh-hash )

################################################################################

list( APPEND utest-mesh-interpolation_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-interpolation_files  utest-mesh-interpolation.cpp )

coolfluid_add_unittest( utest-mesh-interpolation )

################################################################################

list( APPEND utest-mesh-unified-data_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf )
list( APPEND utest-mesh-unified-data_files  utest-mesh-unified-data.cpp )

coolfluid_add_unittest( utest-mesh-unified-data )

################################################################################

list( APPEND utest-mesh-node-element-connectivity_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf )
list( APPEND utest-mesh-node-element-connectivity_files  utest-mesh-node-element-connectivity.cpp )

coolfluid_add_unittest( utest-mesh-node-element-connectivity )

################################################################################

list( APPEND utest-mesh-face-cell-connectivity_cflibs coolfluid_testing coolfluid_mesh_generation coolfluid_mesh_neutral coolfluid_mesh_sf )
list( APPEND utest-mesh-face-cell-connectivity_files  utest-mesh-face-cell-connectivity.cpp )

coolfluid_add_unittest( utest-mesh-face-cell-connectivity )

################################################################################

list( APPEND utest-mesh-construction_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-construction_files  utest-mesh-construction.cpp )

coolfluid_add_unittest( utest-mesh-construction )

################################################################################

list( APPEND utest-mesh-reading_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_sf )
list( APPEND utest-mesh-reading_files  utest-mesh-reading.cpp )

coolfluid_add_unittest( utest-mesh-reading )

################################################################################

list( APPEND utest-mesh-components_cflibs coolfluid_mesh)
list( APPEND utest-mesh-components_files  utest-mesh-components.cpp  )

coolfluid_add_unittest( utest-mesh-components )

################################################################################

list( APPEND utest-mesh-cgns_cflibs      coolfluid_mesh_actions coolfluid_mesh_cgns3 coolfluid_mesh_neutral coolfluid_mesh_gmsh )
list( APPEND utest-mesh-cgns_files       utest-mesh-cgns.cpp )
#list( APPEND utest-mesh-cgns_includedirs ${CGNS_INCLUDE_DIR} )

set( utest-mesh-cgns_condition ${coolfluid_mesh_cgns3_builds} )

coolfluid_add_unittest( utest-mesh-cgns )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-mesh-cgns
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-neu-reader-mpi_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_sf )
list( APPEND utest-neu-reader-mpi_files  utest-neu-reader-mpi.cpp )
  
# Add the target
coolfluid_add_mpi_unittest( utest-neu-reader-mpi 3 )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-neu-reader-mpi
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-gmsh-reader-mpi_cflibs coolfluid_mesh_gmsh coolfluid_mesh_sf )
list( APPEND utest-gmsh-reader-mpi_files  utest-gmsh-reader-mpi.cpp )

# Add the target
coolfluid_add_unittest( utest-gmsh-reader-mpi )

add_custom_command(TARGET utest-gmsh-reader-mpi
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-tg-p1.msh  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-tg-p2.msh  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-qd-p2.msh  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-qd-p2.msh  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-mix-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-mix-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   )

################################################################################

list( APPEND utest-connectivity-data_cflibs coolfluid_mesh_neutral coolfluid_mesh_generation coolfluid_mesh_sf )
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

list( APPEND utest-nodes_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf )
list( APPEND utest-nodes_files  utest-nodes.cpp )

coolfluid_add_unittest( utest-nodes )

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-nodes
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-field_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf coolfluid_mesh_gmsh )
list( APPEND utest-field_files  utest-fields.cpp )

coolfluid_add_unittest( utest-field )

################################################################################

list( APPEND utest-mesh-field_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf coolfluid_mesh_gmsh )
list( APPEND utest-mesh-field_files  utest-mesh-fields.cpp )

coolfluid_add_unittest( utest-mesh-field )

################################################################################

list( APPEND utest-volume-sf_cflibs coolfluid_mesh_sf coolfluid_mesh )
list( APPEND utest-volume-sf_files  utest-volume-sf.cpp )

coolfluid_add_unittest( utest-volume-sf )

################################################################################

list( APPEND utest-vector-benchmark_cflibs coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_mesh_block )
list( APPEND utest-vector-benchmark_files  utest-vector-benchmark.cpp )

set( utest-vector-benchmark_profile ON )

set( utest-vector-benchmark_condition ${CF_TMP_HAVE_SIMPLECOMM})

coolfluid_add_unittest( utest-vector-benchmark )

################################################################################

list( APPEND utest-pt-scotch_cflibs coolfluid_mesh_neutral coolfluid_mesh_sf coolfluid_mesh_gmsh )
#list( APPEND utest-pt-scotch_libs ${PTSCOTCH_LIBRARIES} )

list( APPEND utest-pt-scotch_files  utest-pt-scotch.cpp )
#list( APPEND utest-pt-scotch_includedirs ${PTSCOTCH_INCLUDE_DIR} )

set( utest-pt-scotch_condition ${coolfluid_mesh_ptscotch_builds} )

coolfluid_add_mpi_unittest( utest-pt-scotch 3)

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-pt-scotch
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-zoltan_cflibs coolfluid_mesh_neutral coolfluid_mesh_gmsh coolfluid_mesh_zoltan )
#list( APPEND utest-zoltan_libs ${ZOLTAN_LIBRARIES} )
list( APPEND utest-zoltan_files  utest-zoltan.cpp )
#list( APPEND utest-zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} )

set( utest-zoltan_condition ${coolfluid_mesh_zoltan_builds} )

coolfluid_add_mpi_unittest( utest-zoltan 2)

# Custom post-build rule (must be defined after adding target):
# Copy the file quadtriag.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-zoltan
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/hextet.neu    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-blockmesh-mpi_cflibs coolfluid_mesh coolfluid_mesh_block coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-blockmesh-mpi_files  utest-blockmesh-mpi.cpp )

set( utest-blockmesh-mpi_args ${CF_MPI_TESTS_SIZE} )
set( utest-blockmesh-mpi_scaling ON )

set( utest-blockmesh-mpi_condition ${CF_TMP_HAVE_SIMPLECOMM})

coolfluid_add_mpi_unittest( utest-blockmesh-mpi ${CF_MPI_TESTS_NB_PROCS})

################################################################################

list( APPEND utest-blockmesh-mpi-scale_cflibs coolfluid_mesh_block coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-blockmesh-mpi-scale_files   utest-blockmesh-mpi.cpp )

set( utest-blockmesh-mpi-scale_args "${CF_MPI_TESTS_SIZE} scale" )

set( utest-blockmesh-mpi-scale_scaling ON )

set( utest-blockmesh-mpi-scale_condition ${CF_TMP_HAVE_SIMPLECOMM})

coolfluid_add_mpi_unittest( utest-blockmesh-mpi-scale ${CF_MPI_TESTS_NB_PROCS} )

##########################################################################
# test load mesh wizard
list( APPEND utest-loadmesh_cflibs coolfluid_solver )
list( APPEND utest-loadmesh_files  utest-loadmesh.cpp )

coolfluid_add_unittest( utest-loadmesh )

add_custom_command(TARGET utest-loadmesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-loadmesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

##########################################################################
# test Actions
add_subdirectory( Actions )
