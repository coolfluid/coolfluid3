  CF_FIND_ORPHAN_FILES()

  LIST ( APPEND Testing_files
    Difference.hpp
    ProfiledTestFixture.cpp
    ProfiledTestFixture.hpp
    Timer.hpp
  )

  LIST ( APPEND Testing_includedirs ${GOOGLE_PERFTOOLS_INCLUDE_DIR} )
  LIST ( APPEND Testing_cflibs Common )
  IF(CF_BUILD_GooglePerfTools)
    LIST ( APPEND Testing_libs ${GOOGLE_PERFTOOLS_LIBS} )
  ENDIF()

  CF_ADD_LIBRARY ( Testing )
