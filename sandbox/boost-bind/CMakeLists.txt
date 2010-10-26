list( APPEND sandbox-boost-bind_libs ${Boost_LIBRARIES} )
list( APPEND sandbox-boost-bind_files bind.cpp )

set( sandbox-boost-bind_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-bind )
