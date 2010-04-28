################################################################################

LIST ( APPEND uTestCommon_libs Common )
LIST ( APPEND uTestCommon_files
  UTestCommon.cpp
  TestCPath.cpp
  TestComponent.cpp
  TestConfig.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp  
  TestOSystem.cpp
  TestPE.cpp
  TestStringOps.cpp )

CF_ADD_UNITTEST ( uTestCommon )

################################################################################
