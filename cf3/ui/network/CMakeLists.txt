coolfluid_find_orphan_files()

list( APPEND coolfluid_ui_network_files
  LibNetwork.cpp
  LibNetwork.hpp
  TCPConnection.cpp
  TCPConnection.hpp
  ErrorHandler.hpp
)

coolfluid3_add_library( TARGET coolfluid_ui_network
                        KERNEL
                        SOURCES
                            ${coolfluid_ui_network_files}
                        LIBS
                            coolfluid_common )

