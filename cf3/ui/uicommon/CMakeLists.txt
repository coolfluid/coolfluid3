list( APPEND coolfluid_ui_common_files
  ComponentNames.hpp
  LibUICommon.cpp
  LibUICommon.hpp
  LogMessage.cpp
  LogMessage.hpp )

coolfluid3_add_library( TARGET coolfluid_ui_common
                        KERNEL
						SOURCES ${coolfluid_ui_common_files}
                        LIBS coolfluid_common )
