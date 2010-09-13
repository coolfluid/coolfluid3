  list( APPEND OpenFOAM_files
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

  list( APPEND OpenFOAM_cflibs Mesh SF )

  set( OpenFOAM_kernellib TRUE )

  coolfluid_add_library( OpenFOAM )
  
  add_subdirectory(uTests)
  
