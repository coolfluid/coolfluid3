list( APPEND coolfluid_tools_prowl_files
  LibProwl.hpp
  LibProwl.cpp
  Notifier.hpp
  Notifier.cpp
  prowl/prowl.h
  prowl/prowl.c
)

list( APPEND coolfluid_tools_prowl_cflibs coolfluid_common )
list( APPEND coolfluid_tools_prowl_libs ${OPENSSL_LIBRARIES} )

set( coolfluid_tools_prowl_condition ${OPENSSL_FOUND} )

coolfluid_add_library( coolfluid_tools_prowl )

coolfluid_mark_not_orphan( prowl/main.c )
