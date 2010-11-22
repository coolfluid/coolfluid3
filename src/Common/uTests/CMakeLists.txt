################################################################################
# Test Common

list( APPEND utest-common_cflibs coolfluid_common )
list( APPEND utest-common_files
  utest-common.cpp
  TestOSystem.cpp
  TestStringOps.cpp
  TestHourMinSec.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
)

coolfluid_add_unittest( utest-common )

################################################################################
# Test Component

list( APPEND utest-component_cflibs coolfluid_common )
list( APPEND utest-component_files
  utest-component.cpp
  TestCPath.cpp
  TestComponent.cpp
  DummyComponents.hpp
  TestConfig.cpp
  TestProperties.cpp
  TestSignals.cpp
  TestXmlParams.cpp
  TestStaticSubComponent.cpp
)

coolfluid_add_unittest( utest-component )

################################################################################
# Test Factory

list( APPEND utest-factory_cflibs coolfluid_common )
list( APPEND utest-factory_files
  DummyComponents.hpp
  utest-factory.cpp
)

coolfluid_add_unittest( utest-factory )

################################################################################
# Test Component iterators

list( APPEND utest-component-iterator_cflibs coolfluid_common )
list( APPEND utest-component-iterator_files
  utest-component-iterator.cpp
)

coolfluid_add_unittest( utest-component-iterator )

################################################################################
# Test PE - environment

list( APPEND utest-parallel-environment_cflibs coolfluid_common )
list( APPEND utest-parallel-environment_files
  utest-parallel-environment.cpp
)

coolfluid_add_unittest( utest-parallel-environment )

################################################################################
# Test PE - collective communications

list( APPEND utest-parallel-collective_cflibs coolfluid_common )
list( APPEND utest-parallel-collective_files
  utest-parallel-collective.cpp
#   utest-parallel-alltoall.cpp
)

coolfluid_add_unittest( utest-parallel-collective )

################################################################################
# Test PE - commpattern and objectwrapper

list( APPEND utest-parallel-commpattern_cflibs coolfluid_common )
list( APPEND utest-parallel-commpattern_files
  utest-parallel-commpattern.cpp
)

coolfluid_add_unittest( utest-parallel-commpattern )

