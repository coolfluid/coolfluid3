list( APPEND sandbox-mpi_files mpi.cpp )

set( sandbox-mpi_sandbox_app TRUE )

coolfluid_add_unittest(  sandbox-mpi )
