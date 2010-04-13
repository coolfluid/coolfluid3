################################################################################

 LIST ( APPEND uTestCommon_libs Common )
 LIST ( APPEND uTestCommon_files
  UTestCommon.cxx
  TestLogStream.cxx
  TestOSystem.cxx
  TestCPath.cxx
  TestComponent.cxx
  TestPE.cxx 
  TestStringOps.cxx )

 CF_ADD_UNITTEST ( uTestCommon )

################################################################################
