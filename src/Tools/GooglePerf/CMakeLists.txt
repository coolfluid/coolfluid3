  coolfluid_find_orphan_files()

  list( APPEND coolfluid_google_perftools_files
    GooglePerfProfiling.cpp
    GooglePerfProfiling.hpp
    LibGooglePerfTools.cpp
    LibGooglePerfTools.hpp
  )

  list( APPEND coolfluid_google_perftools_includedirs ${GOOGLE_PERFTOOLS_INCLUDE_DIR} )
  list( APPEND coolfluid_google_perftools_cflibs coolfluid_common )
  list( APPEND coolfluid_google_perftools_libs ${GOOGLE_PERFTOOLS_PROFILER_LIB} )

  coolfluid_add_library( coolfluid_google_perftools )

