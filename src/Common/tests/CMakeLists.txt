LIST ( APPEND CommonTest_libs Common ${Boost_LIBRARIES})
LIST ( APPEND CommonTest_files TestMyInt.cxx)

CF_ADD_UNITTEST(CommonTest)
