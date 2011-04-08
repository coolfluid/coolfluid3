list( APPEND sandbox-boost-mpl_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-boost-mpl_files mpl.cpp )

set( sandbox-boost-mpl_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-mpl )


