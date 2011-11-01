
coolfluid_add_test( UTEST utest-log
                    CPP   utest-log.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-log-level-filter
                    CPP   utest-log-level-filter.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-log-stamp-filter
                    CPP   utest-log-stamp-filter.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-string-ops
                    CPP   utest-string-ops.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-osystem
                    CPP   utest-osystem.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-core
                    CPP   utest-core.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-component
                    CPP   utest-component.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-config
                    CPP   utest-config.cpp DummyComponents.hpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-uri
                    CPP   utest-uri.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-eventhandler
                    CPP   utest-eventhandler.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-properties
                    CPP   utest-properties.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-signals
                    CPP   utest-signals.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-xml-map
                    CPP   utest-xml-map.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-xml-signal-frame
                    CPP   utest-xml-signal-frame.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-xml-signal-options
                    CPP   utest-xml-signal-options.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-static-sub-component
                    CPP   utest-static-sub-component.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-factory
                    CPP   utest-factory.cpp DummyComponents.hpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-component-iterator
                    CPP   utest-component-iterator.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-foreach
                    CPP   utest-foreach.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-cmap
                    CPP   utest-cmap.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-cbuilder
                    CPP   utest-cbuilder.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-create-component-datatype
                    CPP   utest-create-component-datatype.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-options
                    CPP   utest-options.cpp
                    LIBS  coolfluid_common )


coolfluid_add_test( UTEST utest-action-director
                    CPP   utest-action-director.cpp
                    LIBS  coolfluid_common )


################################################################################
# Test PE - environment

coolfluid_add_test( UTEST utest-parallel-environment
                    CPP   utest-parallel-environment.cpp
                    LIBS  coolfluid_common
                    MPI   4 )


coolfluid_add_test( UTEST utest-parallel-collective
                    CPP   utest-parallel-collective.cpp
                          utest-parallel-collective-all_to_all.hpp
                          utest-parallel-collective-all_reduce.hpp
                          utest-parallel-collective-reduce.hpp
                          utest-parallel-collective-scatter.hpp
                          utest-parallel-collective-broadcast.hpp
                          utest-parallel-collective-gather.hpp
                          utest-parallel-collective-all_gather.hpp
                    LIBS  coolfluid_common
                    MPI   4 )


coolfluid_add_test( UTEST utest-parallel-collective-example
                    CPP   utest-parallel-collective-example.cpp
                    LIBS  coolfluid_common
                    MPI   4 )


coolfluid_add_test( UTEST utest-parallel-commwrapper
                    CPP   utest-parallel-commwrapper.cpp
                    LIBS  coolfluid_common
                    MPI   1)


coolfluid_add_test( UTEST utest-parallel-commpattern
                    CPP   utest-parallel-commpattern.cpp
                    LIBS  coolfluid_common
                    MPI   4 )


coolfluid_add_test( UTEST utest-parallel-datatype
                    CPP   utest-parallel-datatype.cpp
                    LIBS  coolfluid_common
                    MPI   4 )


coolfluid_add_test( UTEST utest-parallel-operations
                    CPP   utest-parallel-operations.cpp
                    LIBS  coolfluid_common
                    MPI   4 )

coolfluid_add_test( UTEST utest-common-mpi-buffer
                    CPP   utest-common-mpi-buffer.cpp
                    LIBS  coolfluid_common
                    MPI   4 )

coolfluid_add_test( UTEST utest-build-options
                    CPP   utest-build-options.cpp
                    LIBS  coolfluid_common )

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(PTR_BENCHMARK_SIZE 10000)
else()
  set(PTR_BENCHMARK_SIZE 100)
endif()

coolfluid_add_test( UTEST utest-ptr-benchmark
                    CPP   utest-ptr-benchmark.cpp
                    ARGUMENTS ${PTR_BENCHMARK_SIZE}
                    LIBS  coolfluid_common coolfluid_testing )
