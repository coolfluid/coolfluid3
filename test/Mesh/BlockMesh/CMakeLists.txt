################################################################################

list( APPEND utest-blockmesh-2d_cflibs coolfluid_mesh coolfluid_mesh_blockmesh )
list( APPEND utest-blockmesh-2d_files  utest-blockmesh-2d.cpp )

coolfluid_add_unit_test( utest-blockmesh-2d )

################################################################################

list( APPEND utest-blockmesh-2d-mpi_cflibs coolfluid_mesh coolfluid_mesh_blockmesh )
list( APPEND utest-blockmesh-2d-mpi_files  utest-blockmesh-2d-mpi.cpp )

set(utest-blockmesh-2d-mpi_mpi_test TRUE)
set(utest-blockmesh-2d-mpi_mpi_nprocs 4)

coolfluid_add_unit_test( utest-blockmesh-2d-mpi )

################################################################################

list( APPEND utest-blockmesh-3d-mpi_cflibs coolfluid_mesh coolfluid_mesh_blockmesh coolfluid_mesh_generation coolfluid_testing )
list( APPEND utest-blockmesh-3d-mpi_files  utest-blockmesh-3d-mpi.cpp )
# arguments are number of segments in X, Y, Z directions
list( APPEND utest-blockmesh-3d-mpi_args 12 10 10 )

set(utest-blockmesh-3d-mpi_mpi_test TRUE)
set(utest-blockmesh-3d-mpi_mpi_nprocs 4)

coolfluid_add_unit_test( utest-blockmesh-3d-mpi )

################################################################################

coolfluid_add_python_test(NAME utest-blockmesh-channelgenerator
                          SCRIPT utest-blockmesh-channelgenerator.py
                          UTEST)

################################################################################