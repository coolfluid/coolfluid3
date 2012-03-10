  list( APPEND coolfluid_mesh_cgns3_files
    LibCGNS.cpp
    LibCGNS.hpp
    Shared.hpp
    Shared.cpp
    Reader.hpp
    Reader.cpp
    Writer.hpp
    Writer.cpp
    CGNSExceptions.hpp
  )

  list( APPEND coolfluid_mesh_cgns3_includedirs ${CGNS_INCLUDE_DIRS} )
  list( APPEND coolfluid_mesh_cgns3_libs ${CGNS_LIBRARIES} )
  list( APPEND coolfluid_mesh_cgns3_cflibs coolfluid_mesh_actions )

  set( coolfluid_mesh_cgns3_kernellib TRUE )

  set( coolfluid_mesh_cgns3_condition ${CF3_HAVE_CGNS} )

  coolfluid_add_library( coolfluid_mesh_cgns3 )
