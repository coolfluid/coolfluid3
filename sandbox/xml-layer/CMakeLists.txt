add_definitions( -DNDEBUG )

list( APPEND sandbox-new-xml-layer_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-new-xml-layer_cflibs coolfluid_common )
list( APPEND sandbox-new-xml-layer_files new-xml-layer.cpp )

set( sandbox-new-xml-layer_app TRUE )

coolfluid_add_application( sandbox-new-xml-layer )

list( APPEND sandbox-old-xml-layer_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-old-xml-layer_cflibs coolfluid_common )
list( APPEND sandbox-old-xml-layer_files old-xml-layer.cpp )

set( sandbox-old-xml-layer_app TRUE )

coolfluid_add_application( sandbox-old-xml-layer )


