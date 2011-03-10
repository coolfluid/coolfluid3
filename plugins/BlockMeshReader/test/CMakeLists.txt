################################################################################

list( APPEND utest-block-mesh-reader_cflibs coolfluid_mesh_block coolfluid_mesh_block_reader coolfluid_mesh_neutral coolfluid_meshdiff coolfluid_mesh_sf )
list( APPEND utest-block-mesh-reader_files   utest-block-mesh-reader.cpp )

list( APPEND utest-block-mesh-reader_args ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )

set( utest-block-mesh-reader_condition ${CF_TMP_HAVE_SIMPLECOMM} )

coolfluid_add_unit_test( utest-block-mesh-reader )

################################################################################

list( APPEND dict_files
  cavity2d.dict
  pitzdaily.dict
  channel3d.dict
  uTestBlockMeshReader-Channel3D-reference.neu
  uTestBlockMeshReader-Cavity2D-reference.neu
  uTestBlockMeshReader-PitzDaily-reference.neu
 )

foreach( dfile ${dict_files} )
  add_custom_command(TARGET utest-block-mesh-reader
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()
