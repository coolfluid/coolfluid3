list( APPEND coolfluid_tools_googleperftools_files
    GooglePerfProfiling.cpp
    GooglePerfProfiling.hpp
    LibGooglePerfTools.cpp
    LibGooglePerfTools.hpp
)

list( APPEND coolfluid_tools_googleperftools_cflibs      coolfluid_common )
list( APPEND coolfluid_tools_googleperftools_libs        ${GOOGLEPERFTOOLS_PROFILER_LIBRARY} )
list( APPEND coolfluid_tools_googleperftools_includedirs ${GOOGLEPERFTOOLS_INCLUDE_DIRS} )

set( coolfluid_tools_googleperftools_condition ${CF3_HAVE_GOOGLEPERFTOOLS} )

coolfluid_add_library( coolfluid_tools_googleperftools )
