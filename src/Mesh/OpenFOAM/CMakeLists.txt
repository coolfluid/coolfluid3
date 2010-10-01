  option(CF_STRIP_OPENFOAM_COMPILE_FLAGS "Strip optimization and debug flags for OpenFOAM, to avoid the long compilation" ON)

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

  list( APPEND OpenFOAM_cflibs SF )

  set( OpenFOAM_kernellib TRUE )

  coolfluid_add_library( OpenFOAM )

  if( CMAKE_COMPILER_IS_GNUCC )

    set( CF_OPENFOAM_COMPILE_FLAGS "" )

    # just for this library, disable optimizations
    # to avoid enormous compilation times and memory usage
    if( CF_STRIP_OPENFOAM_COMPILE_FLAGS )
      set(CF_OPENFOAM_COMPILE_FLAGS "${CF_OPENFOAM_COMPILE_FLAGS} -g0 -O0")
    endif()

    set_source_files_properties(Parser.cpp PROPERTIES COMPILE_FLAGS ${CF_OPENFOAM_COMPILE_FLAGS})

  endif()

  add_subdirectory(uTests)

