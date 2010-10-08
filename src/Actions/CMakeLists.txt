coolfluid_find_orphan_files()

list( APPEND Actions_files
  CAction.hpp
  CAction.cpp
  LibActions.hpp
  LibActions.cpp
)

set( Actions_kernellib TRUE )

list( APPEND Actions_libs Mesh )

coolfluid_add_library(Actions)

add_subdirectory(uTests)