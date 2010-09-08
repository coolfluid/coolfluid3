add_definitions( -DNDEBUG )
list( APPEND boost_multiarray_libs ${Boost_LIBRARIES} )
list( APPEND boost_multiarray_files multiarray.cpp )

CF_ADD_APP( boost_multiarray )


