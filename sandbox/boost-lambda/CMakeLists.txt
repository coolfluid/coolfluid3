list( APPEND sandbox-boost-lambda_libs ${Boost_LIBRARIES} )
list( APPEND sandbox-boost-lambda_files lambda.cpp )

coolfluid_add_application( sandbox-boost-lambda )


