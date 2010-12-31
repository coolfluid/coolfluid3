    list( APPEND coolfluid_zoltan_files
      CPartitioner.hpp
      CPartitioner.cpp
      LibZoltan.cpp
      LibZoltan.hpp
    )

    list( APPEND coolfluid_zoltan_cflibs coolfluid_mesh )
    list( APPEND coolfluid_zoltan_libs ${ZOLTAN_LIBRARIES} )
    list( APPEND coolfluid_zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} )

    set( coolfluid_zoltan_kernellib TRUE )

    set( coolfluid_zoltan_condition ${CF_HAVE_ZOLTAN} )

    coolfluid_add_library( coolfluid_zoltan )
