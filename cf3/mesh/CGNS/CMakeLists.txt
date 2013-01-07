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

coolfluid3_add_library( TARGET    coolfluid_mesh_cgns
                        KERNEL
                        SOURCES   ${coolfluid_mesh_cgns_files}
                        LIBS      coolfluid_mesh_actions 
                                  ${CGNS_LIBRARIES}
                        INCLUDES  ${CGNS_INCLUDE_DIRS}
                        CONDITION CF3_HAVE_CGNS )
