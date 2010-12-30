if(CF_HAVE_CGNS)

  list( APPEND coolfluid_mesh_cgns3_files
    LibCGNS.cpp
    LibCGNS.hpp
    Shared.hpp
    Shared.cpp
    CReader.hpp
    CReader.cpp
    CWriter.hpp
    CWriter.cpp
    CGNSExceptions.hpp
  )

  list( APPEND coolfluid_mesh_cgns3_includedirs ${CGNS_INCLUDE_DIR} )
  list( APPEND coolfluid_mesh_cgns3_libs ${CGNS_LIBRARIES} )
  list( APPEND coolfluid_mesh_cgns3_cflibs coolfluid_mesh )

  set( coolfluid_mesh_cgns3_kernellib TRUE )

  coolfluid_add_library( coolfluid_mesh_cgns3 )

else()

  coolfluid_mark_not_orphan(
    LibCGNS.cpp
    LibCGNS.hpp
    Shared.hpp
    Shared.cpp
    CReader.hpp
    CReader.cpp
    CWriter.hpp
    CWriter.cpp
    CGNSExceptions.hpp
  )

endif()
