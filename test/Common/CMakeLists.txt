################################################################################
# Test Log

list( APPEND utest-log_cflibs coolfluid_common )
list( APPEND utest-log_files
  utest-log.cpp
)

coolfluid_add_unittest( utest-log )

################################################################################
# Test Log level filter

list( APPEND utest-log-level-filter_cflibs coolfluid_common )
list( APPEND utest-log-level-filter_files
  utest-log-level-filter.cpp
)

coolfluid_add_unittest( utest-log-level-filter )

################################################################################
# Test Log stamp filter

list( APPEND utest-log-stamp-filter_cflibs coolfluid_common )
list( APPEND utest-log-stamp-filter_files
  utest-log-stamp-filter.cpp
)

coolfluid_add_unittest( utest-log-stamp-filter )

################################################################################
# Test string ops

list( APPEND utest-string-ops_cflibs coolfluid_common )
list( APPEND utest-string-ops_files
  utest-string-ops.cpp
)

coolfluid_add_unittest( utest-string-ops )

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
)

coolfluid_add_unittest( utest-component )

################################################################################
# Test Component

list( APPEND utest-config_cflibs coolfluid_common )
list( APPEND utest-config_files
  DummyComponents.hpp
  utest-config.cpp
)

coolfluid_add_unittest( utest-config )

################################################################################
# Test Component

list( APPEND utest-uri_cflibs coolfluid_common )
list( APPEND utest-uri_files
  utest-uri.cpp
)

coolfluid_add_unittest( utest-uri )

################################################################################
# Test Component

list( APPEND utest-properties_cflibs coolfluid_common )
list( APPEND utest-properties_files
  utest-properties.cpp
)

coolfluid_add_unittest( utest-properties )

################################################################################
# Test Component

list( APPEND utest-signals_cflibs coolfluid_common )
list( APPEND utest-signals_files
  utest-signals.cpp
)

coolfluid_add_unittest( utest-signals )

################################################################################
# Test Component

list( APPEND utest-xml-params_cflibs coolfluid_common )
list( APPEND utest-xml-params_files
  utest-xml-params.cpp
)

coolfluid_add_unittest( utest-xml-params )

################################################################################
# Test Component

list( APPEND utest-static-sub-component_cflibs coolfluid_common )
list( APPEND utest-static-sub-component_files
  utest-static-sub-component.cpp
)

coolfluid_add_unittest( utest-static-sub-component )

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
# Test PE - all_reduce communication

list( APPEND utest-parallel-all_reduce_cflibs coolfluid_common )
list( APPEND utest-parallel-all_reduce_files
  utest-parallel-all_reduce.cpp
)

coolfluid_add_unittest( utest-parallel-all_reduce )

################################################################################
# Test PE - collective type of communications

list( APPEND utest-parallel-collective_cflibs coolfluid_common )
list( APPEND utest-parallel-collective_files
  utest-parallel-collective.cpp
  utest-parallel-collective-all_to_all.hpp
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
