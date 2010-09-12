list( APPEND boost_lambda_libs ${Boost_LIBRARIES} )
list( APPEND boost_lambda_files lambda.cpp )

coolfluid_add_application( boost_lambda )


