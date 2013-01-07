list( APPEND coolfluid_tools_prowl_files
  LibProwl.hpp
  LibProwl.cpp
  Notifier.hpp
  Notifier.cpp
  prowl/prowl.h
  prowl/prowl.c
)

coolfluid3_add_library( TARGET coolfluid_tools_prowl 
                        KERNEL
                        SOURCES   ${coolfluid_tools_prowl_files}
                        LIBS      coolfluid_common
                                  ${OPENSSL_LIBRARIES}
                        CONDITION ${OPENSSL_FOUND} )

coolfluid_mark_not_orphan( prowl/main.c )
