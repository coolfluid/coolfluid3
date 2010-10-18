coolfluid_find_orphan_files()

list( APPEND coolfluid-mesh-transformer_files coolfluid-mesh-transformer.cpp )

list( APPEND coolfluid-mesh-transformer_libs coolfluid_sf coolfluid_neutral coolfluid_gmsh )
if(CF_HAVE_CGNS)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_cgns3 )
endif()
if(CF_BUILD_BlockMeshReader)
  list( APPEND coolfluid-mesh-transformer_libs coolfluid_blockmesh_reader )
endif()

coolfluid_add_application( coolfluid-mesh-transformer )
