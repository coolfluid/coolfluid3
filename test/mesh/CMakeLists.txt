
coolfluid_add_test( UTEST utest-mesh-hash
                    CPP   utest-mesh-hash.cpp
                    LIBS  coolfluid_mesh )


coolfluid_add_test( UTEST utest-mesh-octtree
                    CPP   utest-mesh-octtree.cpp
                    LIBS  coolfluid_mesh_lagrangep1
                    MPI   2 )


coolfluid_add_test( UTEST utest-mesh-stencilcomputerrings
                    CPP   utest-mesh-stencilcomputerrings.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-interpolation
                    CPP   utest-mesh-interpolation.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-unified-data
                    CPP   utest-mesh-unified-data.cpp
                    LIBS  coolfluid_mesh_neu coolfluid_mesh_lagrangep1 
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-node-element-connectivity
                    CPP   utest-mesh-node-element-connectivity.cpp
                    LIBS  coolfluid_mesh_neu coolfluid_mesh_lagrangep1 
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-face-cell-connectivity
                    CPP   utest-mesh-face-cell-connectivity.cpp
                    LIBS  coolfluid_testing coolfluid_mesh_generation coolfluid_mesh_neu coolfluid_mesh_lagrangep1
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-construction
                    CPP   utest-mesh-construction.cpp
                    LIBS  coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 )


coolfluid_add_test( UTEST   utest-mesh-reading
                    CPP     utest-mesh-reading.cpp
                    LIBS    coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST   utest-mesh-transformer
                    CPP     utest-mesh-transformer.cpp
                    LIBS    coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_actions
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-components
                    CPP   utest-mesh-components.cpp
                    LIBS  coolfluid_mesh )


coolfluid_add_test( UTEST     utest-mesh-cgns
                    CPP       utest-mesh-cgns.cpp
                    LIBS      coolfluid_mesh_actions coolfluid_mesh_cgns3 coolfluid_mesh_neu coolfluid_mesh_gmsh 
                    DEPENDS   copy-resources
                    CONDITION coolfluid_mesh_cgns3_builds)


coolfluid_add_test( UTEST   utest-mesh-neu
                    CPP     utest-mesh-neu.cpp
                    LIBS    coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1
                    DEPENDS copy-resources
                    MPI     3)


coolfluid_add_test( UTEST utest-mesh-gmsh
                    CPP   utest-mesh-gmsh.cpp
                    LIBS  coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-tecplot
                    CPP   utest-mesh-tecplot.cpp
                    LIBS  coolfluid_mesh_neu coolfluid_mesh_tecplot coolfluid_mesh_lagrangep1 
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-writemesh
                    CPP   utest-mesh-writemesh.cpp
                    LIBS  coolfluid_mesh_neu coolfluid_mesh_tecplot coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-vtklegacy
                    CPP   utest-vtklegacy-writer.cpp
                    LIBS  coolfluid_mesh_vtklegacy coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )


coolfluid_add_test( UTEST utest-mesh-vtkxml
                    CPP   utest-vtkxml-writer.cpp
                    LIBS  coolfluid_mesh_vtkxml coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )


coolfluid_add_test( UTEST   utest-mesh-connectivity-data
                    CPP     utest-connectivity-data.cpp
                    LIBS    coolfluid_mesh_neu coolfluid_mesh_generation coolfluid_mesh_lagrangep1 
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-hexa3d
                    CPP   utest-mesh-lagrangep1-hexa3d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-tetra3d
                    CPP   utest-mesh-lagrangep1-tetra3d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )

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
list( APPEND utest-nodes_resources ${CF3_RESOURCES_DIR}/quadtriag.neu )
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

set( utest-vector-benchmark_condition ${CF3_TMP_HAVE_SIMPLECOMM})

coolfluid_add_unit_test( utest-vector-benchmark )

################################################################################

list( APPEND utest-pt-scotch_cflibs coolfluid_mesh_ptscotch coolfluid_mesh_neu coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_actions)
#list( APPEND utest-pt-scotch_libs ${PTSCOTCH_LIBRARIES} )

list( APPEND utest-pt-scotch_files  utest-pt-scotch.cpp )
#list( APPEND utest-pt-scotch_includedirs ${PTSCOTCH_INCLUDE_DIR} )

list( APPEND utest-pt-scotch_resources ${CF3_RESOURCES_DIR}/quadtriag.neu ${CF3_RESOURCES_DIR}/hextet.neu)

set( utest-pt-scotch_condition ${coolfluid_mesh_ptscotch_builds} )
set( utest-pt-scotch_mpi_test TRUE )
set( utest-pt-scotch_mpi_nprocs 3 )

coolfluid_add_unit_test( utest-pt-scotch )

################################################################################

list( APPEND utest-zoltan_cflibs coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_zoltan coolfluid_mesh_actions)
#list( APPEND utest-zoltan_libs ${ZOLTAN_LIBRARIES} ${ZOLTAN_EXTRA_LIBRARIES} )
list( APPEND utest-zoltan_files  utest-zoltan.cpp )
#list( APPEND utest-zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} ${ZOLTAN_EXTRA_INCLUDE_DIR} )

list( APPEND utest-zoltan_resources ${CF3_RESOURCES_DIR}/quadtriag.neu ${CF3_RESOURCES_DIR}/hextet.neu)

set( utest-zoltan_condition ${coolfluid_mesh_zoltan_builds} )
set( utest-zoltan_mpi_test TRUE )
set( utest-zoltan_mpi_nprocs 2 )

coolfluid_add_unit_test( utest-zoltan )

################################################################################

# list( APPEND utest-blockmesh-mpi-scale_cflibs coolfluid_mesh_blockmesh coolfluid_mesh_generation coolfluid_mesh_lagrangep1 )
# list( APPEND utest-blockmesh-mpi-scale_files   utest-blockmesh-mpi.cpp )
#
# set( utest-blockmesh-mpi-scale_args "${CF3_MPI_TESTS_SIZE} scale" )
#
# set( utest-blockmesh-mpi-scale_scaling ON )
#
# set( utest-blockmesh-mpi-scale_condition ${CF3_TMP_HAVE_SIMPLECOMM})
# set( utest-blockmesh-mpi-scale_mpi_test TRUE)
#
# coolfluid_add_unit_test( utest-blockmesh-mpi-scale )

##########################################################################
# test parallel field

coolfluid_add_test( UTEST utest-parallel-field
                    CPP utest-parallel-field.cpp
                    LIBS coolfluid_mesh_gmsh
                         coolfluid_mesh_actions
                         coolfluid_mesh_tecplot
                         coolfluid_mesh_lagrangep1
                         coolfluid_mesh_lagrangep2
                    MPI 2
                    CONDITION coolfluid_mesh_zoltan_builds OR coolfluid_mesh_ptscotch_builds )

##########################################################################
# test load mesh wizard
list( APPEND utest-loadmesh_cflibs coolfluid_solver )
list( APPEND utest-loadmesh_files  utest-loadmesh.cpp )
list( APPEND utest-loadmesh_resources ${CF3_RESOURCES_DIR}/rotation-tg-p1.neu ${CF3_RESOURCES_DIR}/rotation-qd-p1.neu)

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
list( APPEND utest-mesh-parallel-overlap_resources ${CF3_RESOURCES_DIR}/sinusbump-tg-p1.msh ${CF3_RESOURCES_DIR}/rotation-tg-p1.neu )

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

coolfluid_add_test( UTEST utest-mesh-deletion
                    CPP   utest-mesh-deletion.cpp
                    LIBS  coolfluid_mesh coolfluid_mesh_generation )

##########################################################################
# test actions
add_subdirectory( actions )

##########################################################################
# test BlockMesh
add_subdirectory(BlockMesh)
