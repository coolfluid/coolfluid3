list( APPEND boost-ublas-gmres_libs ${Boost_LIBRARIES} )
list( APPEND boost-ublas-gmres_files gmres.cpp lin_op.hpp precond.hpp cholesky.hpp orthogonal.hpp )

coolfluid_add_application( boost-ublas-gmres )


