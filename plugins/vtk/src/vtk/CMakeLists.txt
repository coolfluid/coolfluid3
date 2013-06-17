coolfluid3_add_library( TARGET coolfluid_vtk
  SOURCES
    LibVTK.hpp
    LibVTK.cpp
    Tetrahedralize.hpp
    Tetrahedralize.cpp
  LIBS
    coolfluid_mesh
    ${VTK_LIBRARIES}
  INCLUDES
    ${VTK_INCLUDE_DIR}
  CONDITION  VTK_FOUND )