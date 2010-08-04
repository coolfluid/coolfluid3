################################################################################

if (src_Mesh_OpenFOAM_enabled)

  LIST( APPEND uTestBlockMeshReader_libs OpenFOAM )
  LIST( APPEND uTestBlockMeshReader_files
    uTestBlockMeshReader.cpp )

  # note this is not cross-platform
  # because compilation flags are compiler dependent

  SET_SOURCE_FILES_PROPERTIES( uTestBlockMeshReader.cpp
      PROPERTIES COMPILE_FLAGS "-g0 -O0" )

  CF_ADD_UNITTEST( uTestBlockMeshReader )

  add_custom_command(TARGET uTestBlockMeshReader
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/blockMeshDict ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
  )

  
else()

  CF_MARK_NOT_ORPHAN (
    uTestBlockMeshReader.cpp
  )

endif()




################################################################################
