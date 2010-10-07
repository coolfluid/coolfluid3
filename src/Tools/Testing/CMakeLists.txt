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

if( CF_OS_LINUX AND CF_HAVE_RT )

  list( APPEND Testing_libs ${RT_LIB} )

endif()

coolfluid_add_library( Testing )
