################################################################################

 LIST ( APPEND uTestCommon_libs Common )
 LIST ( APPEND uTestCommon_files
  UTestCommon.cxx
  TestLogStream.cxx
  TestPE.cxx )

 CF_ADD_UNITTEST ( uTestCommon )

################################################################################
