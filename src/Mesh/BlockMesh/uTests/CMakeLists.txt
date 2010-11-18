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

