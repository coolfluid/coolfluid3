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
  TestPE.cpp
)

CF_MARK_NOT_ORPHAN (
)


CF_ADD_UNITTEST ( uTestCommon )

################################################################################
