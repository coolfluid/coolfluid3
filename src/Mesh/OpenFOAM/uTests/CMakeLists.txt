################################################################################

LIST ( APPEND uTestBlockMeshReader_libs OpenFOAM )
LIST ( APPEND uTestBlockMeshReader_files
  uTestBlockMeshReader.cpp )

SET_SOURCE_FILES_PROPERTIES(uTestBlockMeshReader.cpp 
    PROPERTIES COMPILE_FLAGS -DTESTFILE_PATH='\"${CMAKE_CURRENT_SOURCE_DIR}\"')

CF_ADD_UNITTEST ( uTestBlockMeshReader )

################################################################################