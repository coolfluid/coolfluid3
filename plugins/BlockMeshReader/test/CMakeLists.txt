################################################################################

list( APPEND utest-block-mesh-reader_cflibs coolfluid_blockmesh coolfluid_blockmesh_reader coolfluid_neutral coolfluid_meshdiff coolfluid_mesh_sf )
list( APPEND utest-block-mesh-reader_files
    utest-block-mesh-reader.cpp )

list( APPEND utest-block-mesh-reader_args ${CMAKE_CURRENT_SOURCE_DIR} )

coolfluid_add_unittest( utest-block-mesh-reader )
