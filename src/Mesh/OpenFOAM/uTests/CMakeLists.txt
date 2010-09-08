################################################################################

  LIST( APPEND uTestBlockMeshReader_cflibs OpenFOAM Neu Gmsh MeshDiff SF )
  LIST( APPEND uTestBlockMeshReader_files
    uTestBlockMeshReader.cpp )

  # avoid long compilation and bloated memory in this specific utest
  IF( CMAKE_COMPILER_IS_GNUCC )
    SET_SOURCE_FILES_PROPERTIES( uTestBlockMeshReader.cpp
      PROPERTIES COMPILE_FLAGS "-g0 -O0" )
  ENDIF()

  CF_ADD_UNITTEST( uTestBlockMeshReader )

  configure_file(channel3d.dict channel3d.dict COPYONLY)
  configure_file(cavity2d.dict cavity2d.dict COPYONLY)
  configure_file(pitzdaily.dict pitzdaily.dict COPYONLY)
  configure_file(uTestBlockMeshReader-Channel3D-reference.neu uTestBlockMeshReader-Channel3D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-Cavity2D-reference.neu uTestBlockMeshReader-Cavity2D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-PitzDaily-reference.neu uTestBlockMeshReader-PitzDaily-reference.neu COPYONLY)

################################################################################
