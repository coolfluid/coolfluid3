  LIST( APPEND OpenFOAM_files
    BlockData.cpp
    BlockData.hpp
    BlockMeshReader.cpp
    BlockMeshReader.hpp
    OpenFOAMAPI.hpp
    Parser.cpp
    Parser.hpp
  )

  LIST( APPEND OpenFOAM_cflibs Mesh )

  SET( OpenFOAM_kernellib TRUE )

  CF_ADD_LIBRARY( OpenFOAM )
  
  ADD_SUBDIRECTORY(uTests)
  
