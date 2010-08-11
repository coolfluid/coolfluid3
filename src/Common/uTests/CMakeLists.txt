################################################################################
# Test Common

LIST( APPEND uTestCommon_cflibs Common )
LIST( APPEND uTestCommon_files
  uTestCommon.cpp
  TestOSystem.cpp
  TestStringOps.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
)

CF_ADD_UNITTEST( uTestCommon )

################################################################################
# Test Component

LIST( APPEND uTestComponent_cflibs Common )
LIST( APPEND uTestComponent_files
  uTestComponent.cpp
  TestCPath.cpp
  TestComponent.cpp
  TestConfig.cpp
  TestSignals.cpp
  TestXmlParams.cpp
)

CF_ADD_UNITTEST( uTestComponent )

################################################################################
# Test Component iterators

LIST( APPEND uTestComponentIteration_cflibs Common )
LIST( APPEND uTestComponentIteration_files
  uTestComponentIteration.cpp
)

CF_ADD_UNITTEST( uTestComponentIteration )

################################################################################
# Test PE

LIST( APPEND uTestPE_cflibs Common )
LIST( APPEND uTestPE_files
  uTestPE.cpp
  TestPE_Finalize.cpp
  TestPECommPattern.cpp
  TestPE_Init.cpp
)

CF_ADD_UNITTEST( uTestPE )

CF_MARK_NOT_ORPHAN( TestPE.cpp )

################################################################################
