list( APPEND coolfluid_mesh_ptscotch_files
      Partitioner.hpp
      Partitioner.cpp
      LibPTScotch.cpp
      LibPTScotch.hpp
    )

list( APPEND coolfluid_mesh_ptscotch_cflibs coolfluid_mesh )
list( APPEND coolfluid_mesh_ptscotch_libs ${PTSCOTCH_LIBRARIES} )
list( APPEND coolfluid_mesh_ptscotch_includedirs ${PTSCOTCH_INCLUDE_DIRS} )

set( coolfluid_mesh_ptscotch_kernellib TRUE )

set( coolfluid_mesh_ptscotch_condition ${CF3_HAVE_PTSCOTCH} )

coolfluid_add_library( coolfluid_mesh_ptscotch )

