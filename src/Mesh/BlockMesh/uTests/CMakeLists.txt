################################################################################

list( APPEND uTestBlockMeshMPI_cflibs BlockMesh MeshGeneration SF )
list( APPEND uTestBlockMeshMPI_files
  uTestBlockMeshMPI.cpp )
set( uTestBlockMeshMPI_args ${CF_MPI_TESTS_SIZE} )
set( uTestBlockMeshMPI_scaling ON )
coolfluid_add_mpi_unittest( uTestBlockMeshMPI )

list( APPEND uTestBlockMeshMPIScale_cflibs BlockMesh MeshGeneration SF )
list( APPEND uTestBlockMeshMPIScale_files
  uTestBlockMeshMPI.cpp )
set( uTestBlockMeshMPIScale_args "${CF_MPI_TESTS_SIZE} scale" )
set( uTestBlockMeshMPIScale_scaling ON )
coolfluid_add_mpi_unittest( uTestBlockMeshMPIScale )

################################################################################
