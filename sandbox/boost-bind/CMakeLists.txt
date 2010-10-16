list( APPEND sandbox-boost-bind_libs ${Boost_LIBRARIES} )
list( APPEND sandbox-boost-bind_files bind.cpp )

coolfluid_add_application( sandbox-boost-bind )
