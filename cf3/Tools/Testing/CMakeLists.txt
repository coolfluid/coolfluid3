list( APPEND coolfluid_testing_files
  Difference.hpp
  LibTesting.cpp
  LibTesting.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
)

list( APPEND coolfluid_testing_libs   ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
list( APPEND coolfluid_testing_cflibs coolfluid_common )

if( CF3_OS_LINUX AND CF3_HAVE_RT )
  list( APPEND coolfluid_testing_libs ${RT_LIB} )
endif()


coolfluid_add_library( coolfluid_testing )
