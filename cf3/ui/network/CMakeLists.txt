coolfluid_find_orphan_files()

list( APPEND coolfluid_ui_network_files
  LibNetwork.cpp
  LibNetwork.hpp
  Connection.cpp
  Connection.hpp 
  ErrorHandler.hpp
)

list( APPEND coolfluid_ui_network_cflibs coolfluid_common )

set( coolfluid_ui_network_kernellib TRUE )

coolfluid_add_library( coolfluid_ui_network )

