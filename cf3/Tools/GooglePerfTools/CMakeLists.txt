list( APPEND coolfluid_tools_googleperftools_files
    GoogleHeapProfiling.cpp
    GoogleHeapProfiling.hpp
    GooglePerfProfiling.cpp
    GooglePerfProfiling.hpp
    LibGooglePerfTools.cpp
    LibGooglePerfTools.hpp
)

coolfluid3_add_library( TARGET    coolfluid_tools_googleperftools 
                        KERNEL
                        SOURCES   ${coolfluid_tools_googleperftools_files} 
                        LIBS      coolfluid_common
                                  ${GOOGLEPERFTOOLS_LIBRARIES}
                        INCLUDES  ${GOOGLEPERFTOOLS_INCLUDE_DIRS}
                        CONDITION CF3_HAVE_GOOGLEPERFTOOLS )
