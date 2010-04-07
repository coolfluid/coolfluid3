################################################################################

 LIST ( APPEND CommonTest_libs Common )
 LIST ( APPEND CommonTest_files
  TestCommon.cxx
  LogStreamTest.cxx
  PETest.cxx )

 CF_ADD_UNITTEST ( CommonTest )

################################################################################
