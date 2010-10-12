list( APPEND boost-bind_libs ${Boost_LIBRARIES} )
list( APPEND boost-bind_files bind.cpp )

coolfluid_add_application( boost-bind )


