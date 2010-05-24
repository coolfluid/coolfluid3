LIST ( APPEND OpenFOAM_files
  BlockMeshReader.hpp
  BlockMeshReader.cpp
)

LIST ( APPEND OpenFOAM_cflibs Mesh )

SET  ( OpenFOAM_kernellib TRUE )

CF_ADD_LIBRARY ( OpenFOAM )

add_subdirectory(uTests)