################################################################################

LIST ( APPEND uTestCommon_libs Common )
LIST ( APPEND uTestCommon_files
  UTestCommon.cpp
  TestPE.cpp
)

CF_MARK_NOT_ORPHAN (
  TestCPath.cpp
  TestComponent.cpp
  TestConfig.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
  TestOSystem.cpp
  TestStringOps.cpp
)


CF_ADD_UNITTEST ( uTestCommon )

################################################################################
