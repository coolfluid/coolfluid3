list( APPEND utest-eigen_files utest-eigen.cpp )
list( APPEND utest-eigen_libs  ${Boost_LIBRARIES} )

coolfluid_add_unittest( utest-eigen )
