SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DCF3_CXX_MANUAL_NATIVE_NULLPTR")

coolfluid3_add_library( TARGET coolfluid_vtk
  SOURCES
    CF3ToVTK.hpp
    CF3ToVTK.cpp
    LibVTK.hpp
    LibVTK.cpp
    MeshInterpolator.hpp
    MeshInterpolator.cpp
    MultiblockWriter.hpp
    MultiblockWriter.cpp
    Tetrahedralize.hpp
    Tetrahedralize.cpp
  LIBS
    coolfluid_mesh
    ${VTK_LIBRARIES}
  INCLUDES
    ${VTK_INCLUDE_DIR}
  CONDITION  VTK_FOUND )
