  list( APPEND OpenFOAM_files
    BlockData.cpp
    BlockData.hpp
    BlockMeshReader.cpp
    BlockMeshReader.hpp
    LibOpenFOAM.cpp
    LibOpenFOAM.hpp
    Parser.cpp
    Parser.hpp
    WriteDict.cpp
    WriteDict.hpp
  )

  list( APPEND OpenFOAM_cflibs Mesh SF )

  set( OpenFOAM_kernellib TRUE )

  coolfluid_add_library( OpenFOAM )

  add_subdirectory(uTests)

