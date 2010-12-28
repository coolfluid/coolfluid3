list( APPEND coolfluid_blockmesh_files
  BlockData.cpp
  BlockData.hpp
  LibBlockMesh.cpp
  LibBlockMesh.hpp
  WriteDict.cpp
  WriteDict.hpp
)

list( APPEND coolfluid_blockmesh_cflibs coolfluid_mesh coolfluid_mesh_sf )

set( coolfluid_blockmesh_kernellib TRUE )


if(CF_TMP_HAVE_SIMPLECOMM)
  coolfluid_add_library( coolfluid_blockmesh )
else()
  coolfluid_mark_not_orphan( ${coolfluid_blockmesh_files} )
endif()
