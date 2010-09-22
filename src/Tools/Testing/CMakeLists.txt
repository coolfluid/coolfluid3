coolfluid_find_orphan_files()

list( APPEND Testing_files
  Difference.hpp
  LibTesting.cpp
  LibTesting.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
)

list( APPEND Testing_cflibs Common )
if( CF_BUILD_GooglePerfTools AND CF_ENABLE_PROFILING )
  list( APPEND Testing_cflibs GooglePerfTools )
endif()

coolfluid_add_library( Testing )
