list( APPEND sandbox-mpi_files mpi.cpp )
list( APPEND sandbox-mpi_libs  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

set( sandbox-mpi_sandbox_app TRUE )

coolfluid_add_application( sandbox-mpi )
