  LIST( APPEND OpenFOAM_files
    BlockData.cpp
    BlockData.hpp
    BlockMeshReader.cpp
    BlockMeshReader.hpp
    OpenFOAMAPI.hpp
    Parser.cpp
    Parser.hpp
    SimpleCommunicationPattern.cpp
    SimpleCommunicationPattern.hpp
    WriteDict.cpp
    WriteDict.hpp
  )

  LIST( APPEND OpenFOAM_cflibs Mesh SF )

  SET( OpenFOAM_kernellib TRUE )

  CF_ADD_LIBRARY( OpenFOAM )
  
  ADD_SUBDIRECTORY(uTests)
  
