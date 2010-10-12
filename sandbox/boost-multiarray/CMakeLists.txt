add_definitions( -DNDEBUG )
list( APPEND boost-multiarray_libs ${Boost_LIBRARIES} )
list( APPEND boost-multiarray_files multiarray.cpp )

coolfluid_add_application( boost-multiarray )


