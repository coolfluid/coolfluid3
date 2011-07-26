list( APPEND sandbox-mpi-solvers-spawning_files mpi-solvers-spawning.cpp )
list( APPEND sandbox-mpi-solvers-spawning_libs  coolfluid_common )

set( sandbox-mpi-solvers-spawning_sandbox_app TRUE )

coolfluid_add_application( sandbox-mpi-solvers-spawning )
