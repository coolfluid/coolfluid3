include_directories(${Trilinos_INCLUDE_DIRS})

list( APPEND ns_semi_implicit_implementation_files_no_debug
  HexasMatrixAssembly.cpp
  PrismsMatrixAssembly.cpp  
  QuadsMatrixAssembly.cpp
  TetrasMatrixAssembly.cpp
  TriagsMatrixAssembly.cpp
)
list( APPEND ns_semi_implicit_implementation_files_with_debug
  NavierStokesSemiImplicit.cpp
)

list( APPEND ns_semi_implicit_implementation_files ${ns_semi_implicit_implementation_files_no_debug} ${ns_semi_implicit_implementation_files_with_debug})

add_library(cf3_ufem_ns_semi_implicit_implementation OBJECT ${ns_semi_implicit_implementation_files})

coolfluid_mark_not_orphan(${ns_semi_implicit_implementation_files})

set_source_files_properties(${ns_semi_implicit_implementation_files_with_debug} PROPERTIES COMPILE_FLAGS "-fPIC")
set_source_files_properties(${ns_semi_implicit_implementation_files_no_debug} PROPERTIES COMPILE_FLAGS "-g0 -fPIC")
