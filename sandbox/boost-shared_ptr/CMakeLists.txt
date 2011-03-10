add_definitions( -DNDEBUG )

list( APPEND sandbox-boost-shared_ptr_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-boost-shared_ptr_cflibs coolfluid_common )
list( APPEND sandbox-boost-shared_ptr_files boost-shared_ptr.cpp )

set( sandbox-boost-shared_ptr_app TRUE )

coolfluid_add_application( sandbox-boost-shared_ptr )

