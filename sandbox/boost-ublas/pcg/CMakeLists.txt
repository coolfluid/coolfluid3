list( APPEND boost-ublas-pcg_libs ${Boost_LIBRARIES} )
list( APPEND boost-ublas-pcg_files pcg.cpp pcg.hpp precond.hpp cholesky.hpp )

coolfluid_add_application( boost-ublas-pcg )


