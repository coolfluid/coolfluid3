################################################################################

list( APPEND utest-blockmesh-mpi_cflibs coolfluid_mesh coolfluid_mesh_block coolfluid_mesh_generation coolfluid_mesh_sf )
list( APPEND utest-blockmesh-mpi_files  utest-blockmesh-mpi.cpp )

set( utest-blockmesh-mpi_args ${CF_MPI_TESTS_SIZE} )
set( utest-blockmesh-mpi_scaling ON )

set( utest-blockmesh-mpi_condition ${CF_TMP_HAVE_SIMPLECOMM})
set( utest-blockmesh-mpi_mpi_test TRUE)
set( utest-blockmesh-mpi_performance_test TRUE)

coolfluid_add_unit_test( utest-blockmesh-mpi )

################################################################################

list( APPEND utest-blockmesh-2d_cflibs coolfluid_mesh coolfluid_mesh_block )
list( APPEND utest-blockmesh-2d_files  utest-blockmesh-2d.cpp )

coolfluid_add_unit_test( utest-blockmesh-2d )

################################################################################

list( APPEND utest-blockmesh-2d-mpi_cflibs coolfluid_mesh coolfluid_mesh_block )
list( APPEND utest-blockmesh-2d-mpi_files  utest-blockmesh-2d-mpi.cpp )

set(utest-blockmesh-2d-mpi_mpi_test TRUE)
set(utest-blockmesh-2d-mpi_mpi_nprocs 4)

coolfluid_add_unit_test( utest-blockmesh-2d-mpi )

################################################################################