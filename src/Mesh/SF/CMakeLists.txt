list( APPEND coolfluid_mesh_sf_files
  Line.hpp
  ETLine1DLagrangeP1.hpp
  ETLine1DLagrangeP1.cpp
  Hexa3DLagrangeP1.cpp
  Hexa3DLagrangeP1.hpp
  LibSF.cpp
  LibSF.hpp
  Line1DLagrangeP1.cpp
  Line1DLagrangeP1.hpp
  SFLineLagrangeP1.cpp
  SFLineLagrangeP1.hpp
  Line2DLagrangeP1.cpp
  Line2DLagrangeP1.hpp
  Line2DLagrangeP2.cpp
  Line2DLagrangeP2.hpp
  Line2DLagrangeP3.cpp
  Line2DLagrangeP3.hpp
  Line3DLagrangeP1.cpp
  Line3DLagrangeP1.hpp
  Point1DLagrangeP1.cpp
  Point1DLagrangeP1.hpp
  Point2DLagrangeP1.cpp
  Point2DLagrangeP1.hpp
  Quad2DLagrangeP1.cpp
  Quad2DLagrangeP1.hpp
  Quad2DLagrangeP2.cpp
  Quad2DLagrangeP2.hpp
  Quad2DLagrangeP3.cpp
  Quad2DLagrangeP3.hpp
  Quad3DLagrangeP1.cpp
  Quad3DLagrangeP1.hpp
  Tetra3DLagrangeP1.cpp
  Tetra3DLagrangeP1.hpp
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
  Triag2DLagrangeP2.cpp
  Triag2DLagrangeP2.hpp
  Triag2DLagrangeP2B.cpp
  Triag2DLagrangeP2B.hpp
  Triag2DLagrangeP3.cpp
  Triag2DLagrangeP3.hpp
  Triag3DLagrangeP1.cpp
  Triag3DLagrangeP1.hpp
  Types.hpp
)

list( APPEND coolfluid_mesh_sf_cflibs coolfluid_mesh )

set( coolfluid_mesh_sf_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_sf )


