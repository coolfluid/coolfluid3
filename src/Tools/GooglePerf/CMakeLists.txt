  coolfluid_find_orphan_files()

  list( APPEND GooglePerfTools_files
    LibGooglePerfTools.cpp
    LibGooglePerfTools.hpp
  )

  list( APPEND GooglePerfTools_includedirs ${GOOGLE_PERFTOOLS_INCLUDE_DIR} )
  list( APPEND GooglePerfTools_cflibs Common )
  list( APPEND GooglePerfTools_libs ${GOOGLE_PERFTOOLS_PROFILER_LIB} )

  coolfluid_add_library( GooglePerfTools )

