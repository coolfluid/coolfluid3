################################################################################

  list( APPEND uTestBlockMeshReader_cflibs OpenFOAM Neu Gmsh MeshDiff SF )
  list( APPEND uTestBlockMeshReader_files
    uTestBlockMeshReader.cpp )
  coolfluid_add_unittest( uTestBlockMeshReader )

  list( APPEND uTestBlockMeshMPI_cflibs OpenFOAM Neu Gmsh MeshDiff SF )
  list( APPEND uTestBlockMeshMPI_files
    uTestBlockMeshMPI.cpp )
  set( uTestBlockMeshMPI_scaling ON )
  coolfluid_add_mpi_unittest( uTestBlockMeshMPI )

  list( APPEND uTestBlockMeshMPIScale_cflibs OpenFOAM Neu Gmsh MeshDiff SF )
  list( APPEND uTestBlockMeshMPIScale_files
    uTestBlockMeshMPIScale.cpp )
  set( uTestBlockMeshMPIScale_scaling ON )
  coolfluid_add_mpi_unittest( uTestBlockMeshMPIScale )
  

  configure_file(channel3d.dict channel3d.dict COPYONLY)
  configure_file(cavity2d.dict cavity2d.dict COPYONLY)
  configure_file(pitzdaily.dict pitzdaily.dict COPYONLY)
  configure_file(uTestBlockMeshReader-Channel3D-reference.neu uTestBlockMeshReader-Channel3D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-Cavity2D-reference.neu uTestBlockMeshReader-Cavity2D-reference.neu COPYONLY)
  configure_file(uTestBlockMeshReader-PitzDaily-reference.neu uTestBlockMeshReader-PitzDaily-reference.neu COPYONLY)

################################################################################
