### Tutorial 1
list( APPEND tut1_files  tut1.cpp  )
list( APPEND tut1_cflibs coolfluid_common)
coolfluid_add_application( tut1 )

### Tutorial 2
list( APPEND tut2_files  tut2.cpp  )
list( APPEND tut2_cflibs coolfluid_common)
coolfluid_add_application( tut2 )

### Tutorial 3
list( APPEND tut3_files  tut3.cpp  )
list( APPEND tut3_cflibs coolfluid_common)
coolfluid_add_application( tut3 )

