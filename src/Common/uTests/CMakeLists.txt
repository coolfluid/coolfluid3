################################################################################
# Test Common

list( APPEND uTestCommon_cflibs Common )
list( APPEND uTestCommon_files
  uTestCommon.cpp
  TestOSystem.cpp
  TestStringOps.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
)

coolfluid_add_unittest( uTestCommon )

################################################################################
# Test Component

list( APPEND uTestComponent_cflibs Common )
list( APPEND uTestComponent_files
  uTestComponent.cpp
  TestCPath.cpp
  TestComponent.cpp
  TestConfig.cpp
  TestProperties.cpp
  TestSignals.cpp
  TestXmlParams.cpp
)

coolfluid_add_unittest( uTestComponent )

################################################################################
# Test Component iterators

list( APPEND uTestComponentIteration_cflibs Common )
list( APPEND uTestComponentIteration_files
  uTestComponentIteration.cpp
)

coolfluid_add_unittest( uTestComponentIteration )

################################################################################
# Test PE

list( APPEND uTestPE_cflibs Common )
list( APPEND uTestPE_files
  uTestPE.cpp
)

coolfluid_add_unittest( uTestPE )

################################################################################
