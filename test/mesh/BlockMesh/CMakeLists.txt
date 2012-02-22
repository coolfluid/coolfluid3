################################################################################

coolfluid_add_test( UTEST  utest-blockmesh-2d
                    CPP    utest-blockmesh-2d.cpp
                    LIBS   coolfluid_mesh coolfluid_mesh_blockmesh )


################################################################################

coolfluid_add_test( UTEST  utest-blockmesh-2d-mpi
                    CPP    utest-blockmesh-2d-mpi.cpp
                    LIBS   coolfluid_mesh coolfluid_mesh_blockmesh
                    MPI    8 )

################################################################################

# arguments are number of segments in X, Y, Z directions
coolfluid_add_test( UTEST     utest-blockmesh-3d-mpi
                    CPP       utest-blockmesh-3d-mpi.cpp
                    LIBS      coolfluid_mesh coolfluid_mesh_blockmesh coolfluid_mesh_generation coolfluid_testing
                    ARGUMENTS 12 10 10
                    MPI       4 )

################################################################################

coolfluid_add_test( UTEST    utest-blockmesh-channelgenerator
                    PYTHON   utest-blockmesh-channelgenerator.py
                    MPI      4 )

################################################################################

coolfluid_add_test( UTEST    utest-blockmesh-generation
                    PYTHON   utest-blockmesh-generation.py )

################################################################################
