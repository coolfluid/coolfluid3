
# Add the project files
list( APPEND coolfluid_ui_common_files
  ComponentNames.hpp
  LibUICommon.cpp
  LibUICommon.hpp
  LogMessage.cpp
  LogMessage.hpp
  NetworkException.cpp
  NetworkException.hpp)

set( coolfluid_ui_common_kernellib TRUE )

# Use Qt Libraries
list( APPEND coolfluid_ui_common_libs ${QT_LIBRARIES} )
list( APPEND coolfluid_ui_common_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_ui_common )
