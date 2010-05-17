################################################################################

LIST ( APPEND uTestCommon_libs Common )
LIST ( APPEND uTestCommon_files
  UTestCommon.cpp
  TestCPath.cpp
  TestComponent.cpp
  TestConfig.cpp
  TestOSystem.cpp
  TestStringOps.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
)

CF_ADD_UNITTEST ( uTestCommon )



LIST ( APPEND uTestPE_libs Common )
LIST ( APPEND uTestPE_files
  uTestPE.cpp
  TestPE_Finalize.cpp
  TestPECommPattern.cpp
  TestPE_Init.cpp
)

CF_ADD_UNITTEST ( uTestPE )



CF_MARK_NOT_ORPHAN (
  TestPE.cpp
)



################################################################################
