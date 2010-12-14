##############################################################################
# BlockMeshReader Plugin
##############################################################################

cmake_minimum_required( VERSION 2.8 FATAL_ERROR )

project( BlockMeshReader CXX )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

option(CF_STRIP_BLOCKMESHREADER_COMPILE_FLAGS "Strip optimization and debug flags for BlockMeshReader, to avoid the long compilation" ON)

if( CF_STRIP_BLOCKMESHREADER_COMPILE_FLAGS AND CMAKE_COMPILER_IS_GNUCC )

    set(CF_BLOCKMESHREADER_COMPILE_FLAGS "${CF_BLOCKMESHREADER_COMPILE_FLAGS} -g0 -O0")

endif()


##############################################################################
# library

list( APPEND coolfluid_blockmesh_reader_files
  BlockMeshReader.cpp
  BlockMeshReader.hpp
  LibBlockMeshReader.cpp
  LibBlockMeshReader.hpp
  Parser.cpp
  Parser.hpp
)

list( APPEND coolfluid_blockmesh_reader_cflibs coolfluid_mesh coolfluid_mesh_generation coolfluid_blockmesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_blockmesh_reader )

if( CF_STRIP_BLOCKMESHREADER_COMPILE_FLAGS AND CMAKE_COMPILER_IS_GNUCC )
  set_source_files_properties(Parser.cpp PROPERTIES COMPILE_FLAGS ${CF_BLOCKMESHREADER_COMPILE_FLAGS})
endif()

##############################################################################
# unit tests

if( CF_ENABLE_UNITTESTS )
  add_subdirectory( test )
endif()

