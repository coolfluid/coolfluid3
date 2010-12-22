list( APPEND sandbox-boost-bind_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-boost-bind_files bind.cpp )

set( sandbox-boost-bind_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-bind )
