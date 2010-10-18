coolfluid_find_orphan_files()

list( APPEND coolfluid_testing_files
  Difference.hpp
  LibTesting.cpp
  LibTesting.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
)

list( APPEND coolfluid_testing_cflibs coolfluid_common )
if( CF_BUILD_GooglePerfTools AND CF_ENABLE_PROFILING )
  list( APPEND coolfluid_testing_cflibs coolfluid_google_perftools )
endif()

if( CF_OS_LINUX AND CF_HAVE_RT )

  list( APPEND coolfluid_testing_libs ${RT_LIB} )

endif()

coolfluid_add_library( coolfluid_testing )
