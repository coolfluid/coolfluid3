list( APPEND coolfluid_mesh_zoltan_files
      Partitioner.hpp
      Partitioner.cpp
      LibZoltan.cpp
      LibZoltan.hpp
    )

list( APPEND coolfluid_mesh_zoltan_cflibs coolfluid_mesh )
list( APPEND coolfluid_mesh_zoltan_libs ${ZOLTAN_LIBRARIES} ${ZOLTAN_EXTRA_LIBRARIES} )
list( APPEND coolfluid_mesh_zoltan_includedirs ${ZOLTAN_INCLUDE_DIRS} ${ZOLTAN_EXTRA_INCLUDE_DIRS} )

set( coolfluid_mesh_zoltan_kernellib TRUE )

set( coolfluid_mesh_zoltan_condition ${CF3_HAVE_ZOLTAN} )

coolfluid_add_library( coolfluid_mesh_zoltan )
