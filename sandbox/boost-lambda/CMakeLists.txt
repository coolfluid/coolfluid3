list( APPEND boost-lambda_libs ${Boost_LIBRARIES} )
list( APPEND boost-lambda_files lambda.cpp )

coolfluid_add_application( boost-lambda )


