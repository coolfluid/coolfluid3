add_definitions( -DNDEBUG )

list( APPEND sandbox-boost-multiarray_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-boost-multiarray_files multiarray.cpp )

set( sandbox-boost-multiarray_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-multiarray )


