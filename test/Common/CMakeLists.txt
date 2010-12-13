################################################################################
# Test Common

list( APPEND utest-common_cflibs coolfluid_common )
list( APPEND utest-common_files
  utest-common.cpp
  TestStringOps.cpp
  TestLog.cpp
  TestLogLevelFilter.cpp
  TestLogStampFilter.cpp
  TestLogStream.cpp
)

coolfluid_add_unittest( utest-common )

################################################################################
# Test OSystem

list( APPEND utest-osystem_cflibs coolfluid_common )
list( APPEND utest-osystem_files
  utest-osystem.cpp
)

coolfluid_add_unittest( utest-osystem )

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
# Test PE - all_to_all communication

list( APPEND utest-parallel-all_to_all_cflibs coolfluid_common )
list( APPEND utest-parallel-all_to_all_files
  utest-parallel-collective.cpp
)

coolfluid_add_unittest( utest-parallel-all_to_all )

################################################################################
# Test PE - all_reduce communication

list( APPEND utest-parallel-collective_cflibs coolfluid_common )
list( APPEND utest-parallel-collective_files
  utest-parallel-all_reduce.cpp
)

coolfluid_add_unittest( utest-parallel-collective )

################################################################################
# Test PE - commpattern and objectwrapper

list( APPEND utest-parallel-commpattern_cflibs coolfluid_common )
list( APPEND utest-parallel-commpattern_files
  utest-parallel-commpattern.cpp
)

coolfluid_add_unittest( utest-parallel-commpattern )

################################################################################
# Test PE - memcheck

list( APPEND utest-parallel-memcheck_cflibs coolfluid_common )
list( APPEND utest-parallel-memcheck_files
  utest-parallel-memcheck.cpp
)

coolfluid_add_unittest( utest-parallel-memcheck )

################################################################################
# Test Foreach - boost_foreach etc.

list( APPEND utest-foreach_cflibs coolfluid_common )
list( APPEND utest-foreach_files
  utest-foreach.cpp
)

coolfluid_add_unittest( utest-foreach )

################################################################################
# Test cmap - boost_cmap etc.

list( APPEND utest-cmap_cflibs coolfluid_common )
list( APPEND utest-cmap_files
  utest-cmap.cpp
)

coolfluid_add_unittest( utest-cmap )
