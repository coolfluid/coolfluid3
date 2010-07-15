  CF_FIND_ORPHAN_FILES()

  LIST ( APPEND GooglePerfTools_files
    GooglePerfTools.cpp
    GooglePerfTools.hpp
  )

  LIST ( APPEND GooglePerfTools_includedirs ${GOOGLE_PERFTOOLS_INCLUDE_DIR} )
  LIST ( APPEND GooglePerfTools_cflibs Common )
  LIST ( APPEND GooglePerfTools_libs ${GOOGLE_PERFTOOLS_PROFILER_LIB} )

  CF_ADD_LIBRARY ( GooglePerfTools )

