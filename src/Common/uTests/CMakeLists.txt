################################################################################
# Test Common

LIST ( APPEND uTestCommon_libs Common )
LIST ( APPEND uTestCommon_files
  uTestCommon.cpp
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
  TestSignals.cpp
  TestXmlParams.cpp
)

CF_ADD_UNITTEST ( uTestCommon )

################################################################################
# Test PE

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
