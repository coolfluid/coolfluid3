option(CF_STRIP_BLOCKMESHREADER_COMPILE_FLAGS "Strip optimization and debug flags for BlockMeshReader, to avoid the long compilation" ON)

list( APPEND BlockMeshReader_files
  BlockMeshReader.cpp
  BlockMeshReader.hpp
  LibBlockMeshReader.cpp
  LibBlockMeshReader.hpp
  Parser.cpp
  Parser.hpp
)

list( APPEND BlockMeshReader_cflibs Mesh MeshGeneration BlockMesh SF )

set( BlockMeshReader_kernellib TRUE )

coolfluid_add_library( BlockMeshReader )

if( CMAKE_COMPILER_IS_GNUCC )

  set( CF_BLOCKMESHREADER_COMPILE_FLAGS "" )

  # just for this library, disable optimizations
  # to avoid enormous compilation times and memory usage
  if( CF_STRIP_BLOCKMESHREADER_COMPILE_FLAGS )
    set(CF_BLOCKMESHREADER_COMPILE_FLAGS "${CF_BLOCKMESHREADER_COMPILE_FLAGS} -g0 -O0")
  endif()

  set_source_files_properties(Parser.cpp PROPERTIES COMPILE_FLAGS ${CF_BLOCKMESHREADER_COMPILE_FLAGS})

endif()

add_subdirectory( uTests )