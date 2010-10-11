
list( APPEND BlockMesh_files
  BlockData.cpp
  BlockData.hpp
  LibBlockMesh.cpp
  LibBlockMesh.hpp
  WriteDict.cpp
  WriteDict.hpp
)

list( APPEND BlockMesh_cflibs Mesh SF )

set( BlockMesh_kernellib TRUE )

coolfluid_add_library( BlockMesh )

add_subdirectory(uTests)

