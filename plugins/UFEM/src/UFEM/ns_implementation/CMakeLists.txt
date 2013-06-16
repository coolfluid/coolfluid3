list( APPEND ns_implementation_files_no_debug
  NavierStokes.cpp
  NavierStokesHexas.cpp
  NavierStokesPrisms.cpp
  NavierStokesQuads.cpp
  NavierStokesTetras.cpp
  NavierStokesTriags.cpp
)
list( APPEND ns_implementation_files_with_debug
)

list( APPEND ns_implementation_files ${ns_implementation_files_no_debug} ${ns_implementation_files_with_debug})

add_library(cf3_ufem_ns_implementation OBJECT ${ns_implementation_files})

coolfluid_mark_not_orphan(${ns_implementation_files})

set_source_files_properties(${ns_implementation_files_with_debug} PROPERTIES COMPILE_FLAGS "-fPIC")
set_source_files_properties(${ns_implementation_files_no_debug} PROPERTIES COMPILE_FLAGS "-g0 -fPIC")
