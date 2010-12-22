################################################################################

list( APPEND utest-block-mesh-reader_cflibs coolfluid_blockmesh coolfluid_blockmesh_reader coolfluid_neutral coolfluid_meshdiff coolfluid_mesh_sf )
list( APPEND utest-block-mesh-reader_files
    utest-block-mesh-reader.cpp )

list( APPEND utest-block-mesh-reader_args ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )

coolfluid_add_unittest( utest-block-mesh-reader )

list( APPEND dict_files  cavity2d.dict  pitzdaily.dict channel3d.dict )
foreach( dfile ${dict_files} )
  add_custom_command(TARGET utest-block-mesh-reader
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()
