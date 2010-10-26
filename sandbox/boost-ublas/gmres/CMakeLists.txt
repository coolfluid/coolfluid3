list( APPEND sandbox-boost-ublas-gmres_libs ${Boost_LIBRARIES} )
list( APPEND sandbox-boost-ublas-gmres_files gmres.cpp lin_op.hpp precond.hpp cholesky.hpp orthogonal.hpp )

set( sandbox-boost-ublas-gmres_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-ublas-gmres )


