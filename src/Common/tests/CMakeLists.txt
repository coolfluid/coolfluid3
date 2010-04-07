 LIST ( APPEND CommonTest_libs Common ${Boost_LIBRARIES})
 LIST ( APPEND CommonTest_files 
   LogStreamTest.cxx)

 CF_ADD_UNITTEST(CommonTest)
