################################################################################

list( APPEND uTestBlockMeshReader_cflibs BlockMesh BlockMeshReader Neu MeshDiff SF )
list( APPEND uTestBlockMeshReader_files
    uTestBlockMeshReader.cpp )

list( APPEND uTestBlockMeshReader_args ${CMAKE_CURRENT_SOURCE_DIR} )

coolfluid_add_unittest( uTestBlockMeshReader )

  