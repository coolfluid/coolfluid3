add_definitions( -DNDEBUG )

list( APPEND sandbox-new-xml-layer_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-new-xml-layer_cflibs coolfluid_common )
list( APPEND sandbox-new-xml-layer_files new-xml-layer.cpp )

set( sandbox-new-xml-layer_app TRUE )

coolfluid_add_application( sandbox-new-xml-layer )

