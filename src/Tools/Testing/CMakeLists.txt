CF_FIND_ORPHAN_FILES()

LIST( APPEND Testing_files
  Difference.hpp
  ProfiledTestFixture.cpp
  ProfiledTestFixture.hpp
  TimedTestFixture.hpp
)

LIST( APPEND Testing_cflibs Common )
IF( CF_BUILD_GooglePerfTools )
  LIST( APPEND Testing_cflibs GooglePerfTools )
ENDIF()

CF_ADD_LIBRARY( Testing )
