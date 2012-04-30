list( APPEND coolfluid_tutorial_files
  LibTutorial.hpp
  LibTutorial.cpp

  Hello.hpp
  Hello.cpp 
)
list( APPEND coolfluid_tutorial_cflibs coolfluid_common)
coolfluid_add_library( coolfluid_tutorial )


### Tutorial 1
list( APPEND tut1_files  tut1.cpp  )
list( APPEND tut1_cflibs coolfluid_common)
coolfluid_add_application( tut1 )

### Tutorial 2
list( APPEND tut2_files  tut2.cpp  )
list( APPEND tut2_cflibs coolfluid_tutorial)
coolfluid_add_application( tut2 )

