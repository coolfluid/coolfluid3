list( APPEND sandbox-boost-lambda_libs ${CF_BOOST_LIBRARIES} )
list( APPEND sandbox-boost-lambda_files lambda.cpp )

set( sandbox-boost-lambda_sandbox_app TRUE )

coolfluid_add_application( sandbox-boost-lambda )


