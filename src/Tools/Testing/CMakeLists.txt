coolfluid_find_orphan_files()

list( APPEND Testing_files
  Difference.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
  TestingAPI.cpp
  TestingAPI.hpp
)

list( APPEND Testing_cflibs Common )
if( CF_BUILD_GooglePerfTools AND CF_ENABLE_PROFILING )
  list( APPEND Testing_cflibs GooglePerfTools )
endif()

coolfluid_add_library( Testing )
