
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


coolfluid_add_test( UTEST utest-mesh-boundingbox
                    CPP   utest-mesh-boundingbox.cpp
                    LIBS  coolfluid_mesh coolfluid_mesh_lagrangep1
                    MPI   2)

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
                    
coolfluid_add_test( UTEST utest-mesh-gmsh-parallel
                    CPP   utest-mesh-gmsh-parallel.cpp
                    LIBS  coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_actions
                    MPI   2
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


coolfluid_add_test( UTEST utest-mesh-lagrangep1-line1d
                    CPP   utest-mesh-lagrangep1-line1d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-line2d
                    CPP   utest-mesh-lagrangep1-line2d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-line3d
                    CPP   utest-mesh-lagrangep1-line3d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-triag2d
                    CPP   utest-mesh-lagrangep1-triag2d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep2b-triag2d
                    CPP   utest-mesh-lagrangep2b-triag2d.cpp
                    LIBS  coolfluid_mesh_lagrangep2b )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-triag3d
                    CPP   utest-mesh-lagrangep1-triag3d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-quad2d
                    CPP   utest-mesh-lagrangep1-quad2d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-lagrangep1-quad3d
                    CPP   utest-mesh-lagrangep1-quad3d.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-matrix-interpolation
                    CPP   utest-matrix-interpolation.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-nodes
                    CPP   utest-nodes.cpp
                    LIBS  coolfluid_mesh_lagrangep1 coolfluid_mesh_neu
                    DEPENDS copy-resources )


coolfluid_add_test( UTEST utest-mesh-dictionary
                    CPP   utest-mesh-dictionary.cpp
                    LIBS  coolfluid_mesh_lagrangep1 )


coolfluid_add_test( UTEST utest-mesh-fieldmanager
                    CPP   utest-mesh-fieldmanager.cpp
                    LIBS  coolfluid_mesh_lagrangep1 coolfluid_mesh_generation )

coolfluid_add_test( UTEST utest-volume-sf
                    CPP   utest-volume-sf.cpp
                    LIBS  coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 )


coolfluid_add_test( PTEST ptest-vector-benchmark
                    CPP   utest-vector-benchmark.cpp
                    LIBS  coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_mesh_blockmesh
                    CONDITION CF3_TMP_HAVE_SIMPLECOMM )

# TODO set profiling ON for this test
# set( utest-vector-benchmark_profile ON )



coolfluid_add_test( UTEST     utest-mesh-ptscotch
                    CPP       utest-mesh-ptscotch.cpp
                    LIBS      coolfluid_mesh_ptscotch coolfluid_mesh_neu coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_actions
                    CONDITION coolfluid_mesh_ptscotch_builds
                    MPI       3
                    DEPENDS   copy-resources )


coolfluid_add_test( UTEST     utest-mesh-zoltan
                    CPP       utest-mesh-zoltan.cpp
                    LIBS      coolfluid_mesh_zoltan coolfluid_mesh_neu coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_actions
                    CONDITION coolfluid_mesh_zoltan_builds
                    MPI       2
                    DEPENDS   copy-resources )


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

coolfluid_add_test( UTEST utest-parallel-field
                    CPP utest-parallel-field.cpp
                    LIBS coolfluid_mesh_gmsh
                         coolfluid_mesh_actions
                         coolfluid_mesh_tecplot
                         coolfluid_mesh_lagrangep1
                         coolfluid_mesh_lagrangep2
                    MPI 2
                    CONDITION coolfluid_mesh_zoltan_builds OR coolfluid_mesh_ptscotch_builds )


coolfluid_add_test( UTEST    utest-mesh-loadmesh
                    CPP      utest-mesh-loadmesh.cpp
                    LIBS     coolfluid_mesh_gmsh coolfluid_mesh_neu
                             coolfluid_mesh_lagrangep1
                             coolfluid_mesh_actions
                    DEPENDS  copy-resources )


coolfluid_add_test( UTEST    utest-mesh-simplemeshgenerator
                    CPP      utest-mesh-simplemeshgenerator.cpp
                    LIBS     coolfluid_mesh
                    MPI      2 )

############################################################################################

set( partitioner_lib "" )
if( coolfluid_mesh_zoltan_builds )
    list( APPEND partitioner_lib coolfluid_mesh_zoltan )
endif()
if( coolfluid_mesh_ptscotch_builds )
    list( APPEND partitioner_lib coolfluid_mesh_ptscotch )
endif()

coolfluid_add_test( UTEST     utest-mesh-parallel-overlap
                    CPP       utest-mesh-parallel-overlap.cpp
                    LIBS      coolfluid_mesh coolfluid_mesh_lagrangep1 coolfluid_mesh_actions ${partitioner_lib} coolfluid_mesh_gmsh coolfluid_mesh_neu coolfluid_mesh_tecplot
                    MPI       2
                    CONDITION coolfluid_mesh_zoltan_builds OR coolfluid_mesh_ptscotch_builds
                    DEPENDS   copy-resources )

############################################################################################

coolfluid_add_test( UTEST     utest-mesh-shapefunctions
                    CPP       utest-mesh-shapefunctions.cpp
                    LIBS      coolfluid_mesh_lagrangep0 coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_lagrangep2b )


coolfluid_add_test( UTEST utest-mesh-deletion
                    CPP   utest-mesh-deletion.cpp
                    LIBS  coolfluid_mesh coolfluid_mesh_generation )

coolfluid_add_test( UTEST utest-mesh-init
                    CPP   utest-mesh-init.cpp
                    LIBS  coolfluid_common )

##########################################################################
# test actions
add_subdirectory( actions )

##########################################################################
# test BlockMesh
add_subdirectory(BlockMesh)
