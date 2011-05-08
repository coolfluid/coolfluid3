add_subdirectory(growl)

coolfluid_mark_not_orphan(
    growl/source/gntp-send++.cpp
    growl/source/growl++.cpp
    growl/source/gntp-send.c
    growl/source/growl.c
    growl/source/md5.c
    growl/source/tcp.c
    growl/headers/growl.h
    growl/headers/md5.h
    growl/headers/msinttypes/inttypes.h
    growl/headers/msinttypes/stdint.h
    growl/headers/tcp.h
    growl/headers/growl++.hpp
)

list( APPEND coolfluid_tools_growl_files
  LibGrowl.hpp 
  LibGrowl.cpp 
  Notifier.hpp
  Notifier.cpp
)

list( APPEND coolfluid_tools_growl_cflibs coolfluid_common growl )

set( coolfluid_tools_growl_kernellib TRUE )

coolfluid_add_library( coolfluid_tools_growl )
