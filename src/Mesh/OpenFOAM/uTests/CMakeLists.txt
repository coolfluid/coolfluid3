################################################################################

  LIST( APPEND uTestBlockMeshReader_cflibs OpenFOAM Neu MeshDiff Mesh SF )
  LIST( APPEND uTestBlockMeshReader_files
    uTestBlockMeshReader.cpp )

  # note this is not cross-platform
  # because compilation flags are compiler dependent

  SET_SOURCE_FILES_PROPERTIES( uTestBlockMeshReader.cpp
      PROPERTIES COMPILE_FLAGS "-g0 -O0" )

  CF_ADD_UNITTEST( uTestBlockMeshReader )

  configure_file(channel3d.dict channel3d.dict COPYONLY)
  configure_file(cavity2d.dict cavity2d.dict COPYONLY)
  configure_file(pitzdaily.dict pitzdaily.dict COPYONLY)
  configure_file(uTestBlockMeshReader-Channel3D-reference.neu uTestBlockMeshReader-Channel3D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-Cavity2D-reference.neu uTestBlockMeshReader-Cavity2D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-PitzDaily-reference.neu uTestBlockMeshReader-PitzDaily-reference.neu COPYONLY)

################################################################################
