list( APPEND coolfluid_googleperftools_files
    GooglePerfProfiling.cpp
    GooglePerfProfiling.hpp
    LibGooglePerfTools.cpp
    LibGooglePerfTools.hpp
)

list( APPEND coolfluid_googleperftools_cflibs      coolfluid_common )
list( APPEND coolfluid_googleperftools_libs        ${GOOGLEPERFTOOLS_PROFILER_LIBRARY} )
list( APPEND coolfluid_googleperftools_includedirs ${GOOGLEPERFTOOLS_INCLUDE_DIR} )

set( coolfluid_googleperftools_condition ${CF_HAVE_GOOGLEPERFTOOLS} )

coolfluid_add_library( coolfluid_googleperftools )
