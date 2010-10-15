################################################################################

list( APPEND utest-block-mesh-reader_cflibs BlockMesh BlockMeshReader Neu MeshDiff SF )
list( APPEND utest-block-mesh-reader_files
    utest-block-mesh-reader.cpp )

list( APPEND utest-block-mesh-reader_args ${CMAKE_CURRENT_SOURCE_DIR} )

coolfluid_add_unittest( utest-block-mesh-reader )
