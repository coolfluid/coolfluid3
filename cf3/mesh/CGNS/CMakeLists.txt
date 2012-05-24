  list( APPEND coolfluid_mesh_cgns_files
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

  list( APPEND coolfluid_mesh_cgns_includedirs ${CGNS_INCLUDE_DIRS} )
  list( APPEND coolfluid_mesh_cgns_libs ${CGNS_LIBRARIES} )
  list( APPEND coolfluid_mesh_cgns_cflibs coolfluid_mesh_actions )

  set( coolfluid_mesh_cgns_kernellib TRUE )

  set( coolfluid_mesh_cgns_condition ${CF3_HAVE_CGNS} )

  coolfluid_add_library( coolfluid_mesh_cgns )
