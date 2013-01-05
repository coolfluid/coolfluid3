list( APPEND coolfluid_testing_files
  Difference.hpp
  LibTesting.cpp
  LibTesting.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
)

coolfluid3_add_library( TARGET  coolfluid_testing
                        KERNEL
                        SOURCES ${coolfluid_testing_files}
                        LIBS    coolfluid_common
                                ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${RT_LIB} )
