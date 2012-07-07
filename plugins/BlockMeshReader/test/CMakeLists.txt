################################################################################

coolfluid_add_test( UTEST      utest-block-mesh-reader
                    CPP        utest-block-mesh-reader.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    LIBS       coolfluid_mesh_blockmesh coolfluid_blockmeshreader coolfluid_mesh_neu coolfluid_meshdiff)

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
