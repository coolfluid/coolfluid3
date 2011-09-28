################################################################################

list( APPEND utest-mesh-hash_cflibs coolfluid_mesh )
list( APPEND utest-mesh-hash_files  utest-mesh-hash.cpp )

coolfluid_add_unit_test( utest-mesh-hash )

################################################################################

list( APPEND utest-mesh-octtree_cflibs coolfluid_mesh_neu coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-octtree_files  utest-mesh-octtree.cpp )

coolfluid_add_unit_test( utest-mesh-octtree )

################################################################################

list( APPEND utest-mesh-stencilcomputerrings_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-stencilcomputerrings_files  utest-mesh-stencilcomputerrings.cpp )

coolfluid_add_unit_test( utest-mesh-stencilcomputerrings )

################################################################################

list( APPEND utest-mesh-interpolation_cflibs coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-interpolation_files  utest-mesh-interpolation.cpp )

coolfluid_add_unit_test( utest-mesh-interpolation )

################################################################################

list( APPEND utest-mesh-unified-data_cflibs coolfluid_mesh_neu coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-unified-data_files  utest-mesh-unified-data.cpp )

coolfluid_add_unit_test( utest-mesh-unified-data )

################################################################################

list( APPEND utest-mesh-node-element-connectivity_cflibs coolfluid_mesh_neu coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-node-element-connectivity_files  utest-mesh-node-element-connectivity.cpp )

coolfluid_add_unit_test( utest-mesh-node-element-connectivity )

################################################################################

list( APPEND utest-mesh-face-cell-connectivity_cflibs coolfluid_testing coolfluid_mesh_generation coolfluid_mesh_neu coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-face-cell-connectivity_files  utest-mesh-face-cell-connectivity.cpp )

coolfluid_add_unit_test( utest-mesh-face-cell-connectivity )

################################################################################

list( APPEND utest-mesh-construction_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 )
list( APPEND utest-mesh-construction_files  utest-mesh-construction.cpp )

coolfluid_add_unit_test( utest-mesh-construction )

################################################################################

list( APPEND utest-mesh-reading_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-reading_files  utest-mesh-reading.cpp )

coolfluid_add_unit_test( utest-mesh-reading )

################################################################################

list( APPEND utest-mesh-transformer_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_actions)
list( APPEND utest-mesh-transformer_files  utest-mesh-transformer.cpp )

coolfluid_add_unit_test( utest-mesh-transformer )

################################################################################

list( APPEND utest-mesh-components_cflibs coolfluid_mesh coolfluid_common)
list( APPEND utest-mesh-components_files  utest-mesh-components.cpp  )

coolfluid_add_unit_test( utest-mesh-components )

################################################################################

list( APPEND utest-mesh-cgns_cflibs      coolfluid_mesh_actions coolfluid_mesh_cgns3 coolfluid_mesh_neu coolfluid_mesh_gmsh )
list( APPEND utest-mesh-cgns_files       utest-mesh-cgns.cpp )
list( APPEND utest-mesh-cgns_includedirs ${CGNS_INCLUDE_DIR} )
list( APPEND utest-mesh-cgns_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)
set( utest-mesh-cgns_condition ${coolfluid_mesh_cgns3_builds} )

coolfluid_add_unit_test( utest-mesh-cgns )

################################################################################

list( APPEND utest-neu-reader-mpi_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )
list( APPEND utest-neu-reader-mpi_files  utest-neu-reader-mpi.cpp )
list( APPEND utest-neu-reader_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)

# Add the target
set( utest-neu-reader-mpi_mpi_test TRUE )
set( utest-neu-reader-mpi_mpi_nprocs 3 )

coolfluid_add_unit_test( utest-neu-reader-mpi 3 )

################################################################################

list( APPEND utest-gmsh-reader-mpi_cflibs coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )
list( APPEND utest-gmsh-reader-mpi_files  utest-gmsh-reader-mpi.cpp )
list( APPEND utest-gmsh-reader-mpi_resources
  ${CF_RESOURCE_DIR}/rectangle-tg-p1.msh
  ${CF_RESOURCE_DIR}/rectangle-tg-p2.msh
  ${CF_RESOURCE_DIR}/rectangle-qd-p2.msh
  ${CF_RESOURCE_DIR}/rectangle-mix-p1.msh
  ${CF_RESOURCE_DIR}/rectangle-mix-p2.msh
)
# Add the target
coolfluid_add_unit_test( utest-gmsh-reader-mpi )

################################################################################

list( APPEND utest-tecplot-writer_cflibs coolfluid_mesh_neu coolfluid_mesh_tecplot coolfluid_mesh_lagrangep1 )
list( APPEND utest-tecplot-writer_files  utest-tecplot-writer.cpp )
list( APPEND utest-tecplot-writer_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)

coolfluid_add_unit_test( utest-tecplot-writer )

################################################################################

list( APPEND utest-mesh-writemesh_cflibs coolfluid_mesh_neu coolfluid_mesh_tecplot coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-writemesh_files  utest-mesh-writemesh.cpp )
list( APPEND utest-mesh-writemesh_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)

coolfluid_add_unit_test( utest-mesh-writemesh )

################################################################################

list( APPEND utest-vtklegacy-writer_cflibs coolfluid_mesh_vtklegacy coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )
list( APPEND utest-vtklegacy-writer_files  utest-vtklegacy-writer.cpp )

coolfluid_add_unit_test( utest-vtklegacy-writer )

################################################################################

list( APPEND utest-vtkxml-writer_cflibs coolfluid_mesh_vtkxml coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )
list( APPEND utest-vtkxml-writer_files  utest-vtkxml-writer.cpp )

coolfluid_add_unit_test( utest-vtkxml-writer )

################################################################################

list( APPEND utest-connectivity-data_cflibs coolfluid_mesh_neu coolfluid_mesh_generation coolfluid_mesh_lagrangep1 )
list( APPEND utest-connectivity-data_files  utest-connectivity-data.cpp )

coolfluid_add_unit_test( utest-connectivity-data )

################################################################################

list( APPEND utest-mesh-lagrangep1-hexa3d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-hexa3d_files utest-mesh-lagrangep1-hexa3d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-hexa3d )

################################################################################

list( APPEND utest-mesh-lagrangep1-tetra3d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-tetra3d_files utest-mesh-lagrangep1-tetra3d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-tetra3d )

################################################################################

list( APPEND utest-mesh-lagrangep1-line1d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-line1d_files utest-mesh-lagrangep1-line1d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-line1d )

################################################################################

list( APPEND utest-mesh-lagrangep1-line2d_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )
list( APPEND utest-mesh-lagrangep1-line2d_files utest-mesh-lagrangep1-line2d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-line2d )

################################################################################

list( APPEND utest-mesh-lagrangep1-line3d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-line3d_files utest-mesh-lagrangep1-line3d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-line3d )

################################################################################

list( APPEND utest-mesh-lagrangep1-triag2d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-triag2d_files utest-mesh-lagrangep1-triag2d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-triag2d )

################################################################################

list( APPEND utest-mesh-lagrangep2b-triag2d_cflibs coolfluid_mesh_lagrangep2b )
list( APPEND utest-mesh-lagrangep2b-triag2d_files utest-mesh-lagrangep2b-triag2d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep2b-triag2d )

################################################################################

list( APPEND utest-mesh-lagrangep1-triag3d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-triag3d_files utest-mesh-lagrangep1-triag3d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-triag3d )

################################################################################

list( APPEND utest-mesh-lagrangep1-quad2d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-quad2d_files utest-mesh-lagrangep1-quad2d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-quad2d )

################################################################################

list( APPEND utest-mesh-lagrangep1-quad3d_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-lagrangep1-quad3d_files utest-mesh-lagrangep1-quad3d.cpp )

coolfluid_add_unit_test( utest-mesh-lagrangep1-quad3d )

################################################################################

list( APPEND utest-matrix-interpolation_cflibs  coolfluid_mesh_lagrangep1 )
list( APPEND utest-matrix-interpolation_files   utest-matrix-interpolation.cpp )

coolfluid_add_unit_test( utest-matrix-interpolation )

################################################################################

list( APPEND utest-nodes_cflibs coolfluid_mesh_neu coolfluid_mesh_lagrangep1 )
list( APPEND utest-nodes_files  utest-nodes.cpp )
list( APPEND utest-nodes_resources ${CF_RESOURCE_DIR}/quadtriag.neu )
coolfluid_add_unit_test( utest-nodes )

################################################################################

list( APPEND utest-mesh-fieldgroup_cflibs coolfluid_mesh_lagrangep1 )
list( APPEND utest-mesh-fieldgroup_files  utest-mesh-fieldgroup.cpp )

coolfluid_add_unit_test( utest-mesh-fieldgroup )

################################################################################

list( APPEND utest-mesh-fieldmanager_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_generation)
list( APPEND utest-mesh-fieldmanager_files  utest-mesh-fieldmanager.cpp )

coolfluid_add_unit_test( utest-mesh-fieldmanager )

################################################################################

list( APPEND utest-volume-sf_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3)
list( APPEND utest-volume-sf_files  utest-volume-sf.cpp )

coolfluid_add_unit_test( utest-volume-sf )

################################################################################

list( APPEND utest-vector-benchmark_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_mesh_blockmesh )
list( APPEND utest-vector-benchmark_files  utest-vector-benchmark.cpp )

set( utest-vector-benchmark_profile ON )
set( utest-vector-benchmark_performance_test TRUE )

set( utest-vector-benchmark_condition ${CF_TMP_HAVE_SIMPLECOMM})

coolfluid_add_unit_test( utest-vector-benchmark )

################################################################################

list( APPEND utest-pt-scotch_cflibs coolfluid_mesh_ptscotch coolfluid_mesh_neu coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_actions)
#list( APPEND utest-pt-scotch_libs ${PTSCOTCH_LIBRARIES} )

list( APPEND utest-pt-scotch_files  utest-pt-scotch.cpp )
#list( APPEND utest-pt-scotch_includedirs ${PTSCOTCH_INCLUDE_DIR} )

list( APPEND utest-pt-scotch_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)

set( utest-pt-scotch_condition ${coolfluid_mesh_ptscotch_builds} )
set( utest-pt-scotch_mpi_test TRUE )
set( utest-pt-scotch_mpi_nprocs 3 )

coolfluid_add_unit_test( utest-pt-scotch )

################################################################################

list( APPEND utest-zoltan_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_zoltan coolfluid_mesh_actions)
#list( APPEND utest-zoltan_libs ${ZOLTAN_LIBRARIES} ${ZOLTAN_EXTRA_LIBRARIES} )
list( APPEND utest-zoltan_files  utest-zoltan.cpp )
#list( APPEND utest-zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} ${ZOLTAN_EXTRA_INCLUDE_DIR} )

list( APPEND utest-zoltan_resources ${CF_RESOURCE_DIR}/quadtriag.neu ${CF_RESOURCE_DIR}/hextet.neu)

set( utest-zoltan_condition ${coolfluid_mesh_zoltan_builds} )
set( utest-zoltan_mpi_test TRUE )
set( utest-zoltan_mpi_nprocs 2 )

coolfluid_add_unit_test( utest-zoltan )

################################################################################

# list( APPEND utest-blockmesh-mpi-scale_cflibs coolfluid_mesh_blockmesh coolfluid_mesh_generation coolfluid_mesh_lagrangep1 )
# list( APPEND utest-blockmesh-mpi-scale_files   utest-blockmesh-mpi.cpp )
#
# set( utest-blockmesh-mpi-scale_args "${CF_MPI_TESTS_SIZE} scale" )
#
# set( utest-blockmesh-mpi-scale_scaling ON )
#
# set( utest-blockmesh-mpi-scale_condition ${CF_TMP_HAVE_SIMPLECOMM})
# set( utest-blockmesh-mpi-scale_mpi_test TRUE)
#
# coolfluid_add_unit_test( utest-blockmesh-mpi-scale )

##########################################################################
# test parallel field

list( APPEND utest-parallel-field_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_actions coolfluid_mesh_zoltan)
list( APPEND utest-parallel-field_files  utest-parallel-field.cpp )

set( utest-parallel-field_condition ${coolfluid_mesh_zoltan_builds} )
set( utest-parallel-field_mpi_test TRUE )
set( utest-parallel-field_mpi_nprocs 2 )

coolfluid_add_unit_test( utest-parallel-field )


##########################################################################
# test load mesh wizard
list( APPEND utest-loadmesh_cflibs coolfluid_solver )
list( APPEND utest-loadmesh_files  utest-loadmesh.cpp )
list( APPEND utest-loadmesh_resources ${CF_RESOURCE_DIR}/rotation-tg-p1.neu ${CF_RESOURCE_DIR}/rotation-qd-p1.neu)

coolfluid_add_unit_test( utest-loadmesh )

##########################################################################
# test simple mesh generator
list( APPEND utest-mesh-simplemeshgenerator_cflibs coolfluid_mesh )
list( APPEND utest-mesh-simplemeshgenerator_files  utest-mesh-simplemeshgenerator.cpp )

set( utest-mesh-simplemeshgenerator_mpi_test   TRUE)
set( utest-mesh-simplemeshgenerator_mpi_nprocs 2 )

coolfluid_add_unit_test( utest-mesh-simplemeshgenerator )

##########################################################################
# test parallel overlap

list( APPEND utest-mesh-parallel-overlap_cflibs coolfluid_mesh coolfluid_mesh_lagrangep1 coolfluid_mesh_actions coolfluid_mesh_zoltan coolfluid_mesh_gmsh coolfluid_mesh_neu coolfluid_mesh_tecplot )
list( APPEND utest-mesh-parallel-overlap_files  utest-mesh-parallel-overlap.cpp )
list( APPEND utest-mesh-parallel-overlap_resources ${CF_RESOURCE_DIR}/sinusbump-tg-p1.msh ${CF_RESOURCE_DIR}/rotation-tg-p1.neu )

set( utest-mesh-parallel-overlap_condition ${coolfluid_mesh_zoltan_builds} )
set( utest-mesh-parallel-overlap_mpi_test   TRUE)
set( utest-mesh-parallel-overlap_mpi_nprocs 2 )

coolfluid_add_unit_test( utest-mesh-parallel-overlap )

##########################################################################
# test simple mesh generator
list( APPEND utest-mesh-shapefunctions_cflibs coolfluid_mesh_lagrangep0 coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_lagrangep2b)
list( APPEND utest-mesh-shapefunctions_files  utest-mesh-shapefunctions.cpp )

coolfluid_add_unit_test( utest-mesh-shapefunctions )

##########################################################################
# test Actions
add_subdirectory( Actions )

##########################################################################
# test BlockMesh
add_subdirectory(BlockMesh)
