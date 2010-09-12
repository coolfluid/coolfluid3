list( APPEND boost_bind_libs ${Boost_LIBRARIES} )
list( APPEND boost_bind_files bind.cpp )

coolfluid_add_application( boost_bind )


