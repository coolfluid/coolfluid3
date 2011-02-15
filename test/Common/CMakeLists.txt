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

coolfluid_add_mpi_unittest( utest-parallel-environment ${CF_MPI_TESTS_NB_PROCS} )

################################################################################
# Test PE - collective type of communications

list( APPEND utest-parallel-collective_cflibs coolfluid_common )
list( APPEND utest-parallel-collective_files
  utest-parallel-collective.cpp
  utest-parallel-collective-all_to_all.hpp
  utest-parallel-collective-all_reduce.hpp
  utest-parallel-collective-reduce.hpp
  utest-parallel-collective-scatter.hpp
)

coolfluid_add_mpi_unittest( utest-parallel-collective ${CF_MPI_TESTS_NB_PROCS} )

################################################################################
# Test PE - commpattern and objectwrapper

list( APPEND utest-parallel-commpattern_cflibs coolfluid_common )
list( APPEND utest-parallel-commpattern_files
  utest-parallel-commpattern.cpp
)

coolfluid_add_mpi_unittest( utest-parallel-commpattern ${CF_MPI_TESTS_NB_PROCS} )

################################################################################
# Test PE - datatype

list( APPEND utest-parallel-datatype_cflibs coolfluid_common )
list( APPEND utest-parallel-datatype_files
  utest-parallel-datatype.cpp
)

coolfluid_add_mpi_unittest( utest-parallel-datatype ${CF_MPI_TESTS_NB_PROCS} )

################################################################################
# Test PE - operations

list( APPEND utest-parallel-operations_cflibs coolfluid_common )
list( APPEND utest-parallel-operations_files
  utest-parallel-operations.cpp
)

coolfluid_add_mpi_unittest( utest-parallel-operations ${CF_MPI_TESTS_NB_PROCS} )

################################################################################
# Test Foreach - boost_foreach etc.

list( APPEND utest-foreach_cflibs coolfluid_common )
list( APPEND utest-foreach_files
  utest-foreach.cpp
)

coolfluid_add_unittest( utest-foreach )

################################################################################
# Test cmap

list( APPEND utest-cmap_cflibs coolfluid_common )
list( APPEND utest-cmap_files
  utest-cmap.cpp
)

coolfluid_add_unittest( utest-cmap )

################################################################################
# Test DataComponentWrapper

list( APPEND utest-create-component-datatype_cflibs coolfluid_common )
list( APPEND utest-create-component-datatype_files
  utest-create-component-datatype.cpp
)

coolfluid_add_unittest( utest-create-component-datatype )
