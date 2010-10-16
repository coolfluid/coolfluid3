add_definitions( -DNDEBUG )
list( APPEND sandbox-boost-multiarray_libs ${Boost_LIBRARIES} )
list( APPEND sandbox-boost-multiarray_files multiarray.cpp )

coolfluid_add_application( sandbox-boost-multiarray )


