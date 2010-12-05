if( ${CF_HAVE_PTSCOTCH} )
    list( APPEND coolfluid_ptscotch_files
      CPartitioner.hpp
      CPartitioner.cpp
      LibPTScotch.cpp
      LibPTScotch.hpp
    )

    list( APPEND coolfluid_ptscotch_cflibs coolfluid_mesh )
    list( APPEND coolfluid_ptscotch_libs ${PTSCOTCH_LIBRARIES} )
    list( APPEND coolfluid_ptscotch_includedirs ${PTSCOTCH_INCLUDE_DIR} )

    set( coolfluid_ptscotch_kernellib TRUE )

    coolfluid_add_library( coolfluid_ptscotch )
    
else()
  coolfluid_mark_not_orphan( CPartitioner.hpp
                             CPartitioner.cpp
                             LibPTScotch.cpp
                             LibPTScotch.hpp )
endif()

