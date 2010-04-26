################################################################################

LIST ( APPEND uTestCommon_libs Common )
LIST ( APPEND uTestCommon_files
  UTestCommon.cpp
  TestHourMinSec.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp  
  TestOSystem.cpp
  TestCPath.cpp
  TestComponent.cpp
  TestPE.cpp
  TestStringOps.cpp )

CF_ADD_UNITTEST ( uTestCommon )

################################################################################
