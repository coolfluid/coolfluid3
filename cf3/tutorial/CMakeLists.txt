coolfluid3_add_library( TARGET coolfluid_tutorial
                        TEST
                        SOURCES
                           LibTutorial.hpp
                           LibTutorial.cpp
                           Hello.hpp
                           Hello.cpp
                        LIBS  coolfluid_common )

### Tutorial 1
list( APPEND tut1_files  tut1.cpp  )
list( APPEND tut1_cflibs coolfluid_common)
coolfluid_add_application( tut1 )

### Tutorial 2
list( APPEND tut2_files  tut2.cpp  )
list( APPEND tut2_cflibs coolfluid_tutorial)
coolfluid_add_application( tut2 )

