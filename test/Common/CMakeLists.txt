################################################################################
# Test Log

list( APPEND utest-log_cflibs coolfluid_common )
list( APPEND utest-log_files
  utest-log.cpp
)

coolfluid_add_unit_test( utest-log )

################################################################################
# Test Log level filter

list( APPEND utest-log-level-filter_cflibs coolfluid_common )
list( APPEND utest-log-level-filter_files
  utest-log-level-filter.cpp
)

coolfluid_add_unit_test( utest-log-level-filter )

################################################################################
# Test Log stamp filter

list( APPEND utest-log-stamp-filter_cflibs coolfluid_common )
list( APPEND utest-log-stamp-filter_files
  utest-log-stamp-filter.cpp
)

coolfluid_add_unit_test( utest-log-stamp-filter )

################################################################################
# Test string ops

list( APPEND utest-string-ops_cflibs coolfluid_common )
list( APPEND utest-string-ops_files
  utest-string-ops.cpp
)

coolfluid_add_unit_test( utest-string-ops )

################################################################################
# Test OSystem

list( APPEND utest-osystem_cflibs coolfluid_common )
list( APPEND utest-osystem_files
  utest-osystem.cpp
)

coolfluid_add_unit_test( utest-osystem )

################################################################################
# Test Core

list( APPEND utest-core_cflibs coolfluid_common )
list( APPEND utest-core_files
  utest-core.cpp
)

coolfluid_add_unit_test( utest-core )

################################################################################
# Test Component

list( APPEND utest-component_cflibs coolfluid_common )
list( APPEND utest-component_files
  utest-component.cpp
)

coolfluid_add_unit_test( utest-component )

################################################################################
# Test Component

list( APPEND utest-config_cflibs coolfluid_common )
list( APPEND utest-config_files
  DummyComponents.hpp
  utest-config.cpp
)

coolfluid_add_unit_test( utest-config )

################################################################################
# Test Component

list( APPEND utest-uri_cflibs coolfluid_common )
list( APPEND utest-uri_files
  utest-uri.cpp
)

coolfluid_add_unit_test( utest-uri )

################################################################################
# Test Component

list( APPEND utest-properties_cflibs coolfluid_common )
list( APPEND utest-properties_files
  utest-properties.cpp
)

coolfluid_add_unit_test( utest-properties )

################################################################################
# Test Component

list( APPEND utest-signals_cflibs coolfluid_common )
list( APPEND utest-signals_files
  utest-signals.cpp
)

coolfluid_add_unit_test( utest-signals )

################################################################################
# Test Component

list( APPEND utest-xml-map_cflibs coolfluid_common )
list( APPEND utest-xml-map_files
  utest-xml-map.cpp
)

coolfluid_add_unit_test( utest-xml-map )

################################################################################
# Test Component

list( APPEND utest-xml-signal-frame_cflibs coolfluid_common )
list( APPEND utest-xml-signal-frame_files
  utest-xml-signal-frame.cpp
)

coolfluid_add_unit_test( utest-xml-signal-frame )

################################################################################
# Test Component

list( APPEND utest-static-sub-component_cflibs coolfluid_common )
list( APPEND utest-static-sub-component_files
  utest-static-sub-component.cpp
)

coolfluid_add_unit_test( utest-static-sub-component )

################################################################################
# Test Factory

list( APPEND utest-factory_cflibs coolfluid_common )
list( APPEND utest-factory_files
  DummyComponents.hpp
  utest-factory.cpp
)

coolfluid_add_unit_test( utest-factory )

################################################################################
# Test Component iterators

list( APPEND utest-component-iterator_cflibs coolfluid_common )
list( APPEND utest-component-iterator_files
  utest-component-iterator.cpp
)

coolfluid_add_unit_test( utest-component-iterator )

################################################################################
# Test PE - environment

list( APPEND utest-parallel-environment_cflibs coolfluid_common )
list( APPEND utest-parallel-environment_files
  utest-parallel-environment.cpp
)

set(utest-parallel-environment_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-environment )

################################################################################
# Test PE - collective type of communications

list( APPEND utest-parallel-collective_cflibs coolfluid_common )
list( APPEND utest-parallel-collective_files
  utest-parallel-collective.cpp
  utest-parallel-collective-all_to_all.hpp
  utest-parallel-collective-all_reduce.hpp
  utest-parallel-collective-reduce.hpp
  utest-parallel-collective-scatter.hpp
  utest-parallel-collective-broadcast.hpp
  utest-parallel-collective-gather.hpp
  utest-parallel-collective-all_gather.hpp
)

set(utest-parallel-collective_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-collective )

################################################################################
# Test PE - parallel collective example

list( APPEND utest-parallel-collective-example_cflibs coolfluid_common )
list( APPEND utest-parallel-collective-example_files
  utest-parallel-collective-example.cpp
)

set( utest-parallel-collective-example_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-collective-example )

################################################################################
# Test PE - commpattern and objectwrapper

list( APPEND utest-parallel-commpattern_cflibs coolfluid_common )
list( APPEND utest-parallel-commpattern_files
  utest-parallel-commpattern.cpp
)

set(utest-parallel-commpattern_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-commpattern )

################################################################################
# Test PE - datatype

list( APPEND utest-parallel-datatype_cflibs coolfluid_common )
list( APPEND utest-parallel-datatype_files
  utest-parallel-datatype.cpp
)

set(utest-parallel-datatype_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-datatype )

################################################################################
# Test PE - operations

list( APPEND utest-parallel-operations_cflibs coolfluid_common )
list( APPEND utest-parallel-operations_files
  utest-parallel-operations.cpp
)

set( utest-parallel-operations_mpi_test TRUE)
coolfluid_add_unit_test( utest-parallel-operations )

################################################################################
# Test PE - buffer

list( APPEND utest-common-mpi-buffer_cflibs coolfluid_common )
list( APPEND utest-common-mpi-buffer_files
  utest-common-mpi-buffer.cpp
)

set(utest-common-mpi-buffer_mpi_test TRUE)
coolfluid_add_unit_test( utest-common-mpi-buffer )

################################################################################
# Test Foreach - boost_foreach etc.

list( APPEND utest-foreach_cflibs coolfluid_common )
list( APPEND utest-foreach_files
  utest-foreach.cpp
)

coolfluid_add_unit_test( utest-foreach )

################################################################################
# Test cmap

list( APPEND utest-cmap_cflibs coolfluid_common )
list( APPEND utest-cmap_files
  utest-cmap.cpp
)

coolfluid_add_unit_test( utest-cmap )

################################################################################
# Test cbuilder

list( APPEND utest-cbuilder_cflibs coolfluid_common )
list( APPEND utest-cbuilder_files
  utest-cbuilder.cpp
)

coolfluid_add_unit_test( utest-cbuilder )

################################################################################
# Test DataComponentWrapper

list( APPEND utest-create-component-datatype_cflibs coolfluid_common )
list( APPEND utest-create-component-datatype_files
  utest-create-component-datatype.cpp
)

coolfluid_add_unit_test( utest-create-component-datatype )

################################################################################
# Test options

list( APPEND utest-options_cflibs coolfluid_common )
list( APPEND utest-options_files
  utest-options.cpp
)

coolfluid_add_unit_test( utest-options )

################################################################################
# Test CActionDirector

list( APPEND utest-action-director_cflibs coolfluid_common )
list( APPEND utest-action-director_files
  utest-action-director.cpp
)

coolfluid_add_unit_test( utest-action-director )