include ( FindGooglePerftools )

if(NOT CF_HAVE_GOOGLE_PERFTOOLS)
    message(FATAL_ERROR "Google Perftools not found, required for plugin GooglePerfTools")
endif()

LIST ( APPEND GooglePerfTools_files
GooglePerfTools.cpp
GooglePerfTools.hpp
)

LIST ( APPEND GooglePerfTools_includedirs ${GOOGLE_PERFTOOLS_INCLUDE_DIR} )
LIST ( APPEND GooglePerfTools_cflibs Common )
LIST ( APPEND GooglePerfTools_libs ${GOOGLE_PERFTOOLS_LIBRARY} )

CF_ADD_LIBRARY ( GooglePerfTools )
