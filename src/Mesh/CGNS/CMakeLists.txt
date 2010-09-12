if(CF_HAVE_CGNS)

  list( APPEND CGNS3_files
    CGNSAPI.hpp
    Shared.hpp
    Shared.cpp
    CReader.hpp
    CReader.cpp
    CWriter.hpp
    CWriter.cpp
    CGNSExceptions.hpp
  )

  list( APPEND CGNS3_includedirs ${CGNS_INCLUDE_DIR} )
  list( APPEND CGNS3_libs ${CGNS_LIBRARY} )
  list( APPEND CGNS3_cflibs Mesh )

  set( CGNS3_kernellib TRUE )

  coolfluid_add_library ( CGNS3 )

else()

  coolfluid_mark_not_orphan (
    CGNSAPI.hpp
    Shared.hpp
    Shared.cpp
    CReader.hpp
    CReader.cpp
    CWriter.hpp
    CWriter.cpp
    CGNSExceptions.hpp
  )

endif()
