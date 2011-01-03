
# Add the project files
list( APPEND coolfluid_network_files
  ComponentNames.hpp
  ComponentType.cpp
  ComponentType.hpp
  LibNetwork.cpp
  LibNetwork.hpp
  LogMessage.cpp
  LogMessage.hpp
  NetworkException.cpp
  NetworkException.hpp)

set( coolfluid_network_kernellib TRUE )

# Use Qt Libraries
list( APPEND coolfluid_network_libs ${QT_LIBRARIES} )
list( APPEND coolfluid_network_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_network )
