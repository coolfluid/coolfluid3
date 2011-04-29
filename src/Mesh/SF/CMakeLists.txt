list( APPEND coolfluid_mesh_sf_files
  Hexa.hpp
  Hexa3DLagrangeP1.cpp
  Hexa3DLagrangeP1.hpp
  LibSF.cpp
  LibSF.hpp
  Line1DLagrangeP1.cpp
  Line1DLagrangeP1.hpp
  Line.hpp
  Line2DLagrangeP1.cpp
  Line2DLagrangeP1.hpp
  Line2DLagrangeP2.cpp
  Line2DLagrangeP2.hpp
  Line2DLagrangeP3.cpp
  Line2DLagrangeP3.hpp
  Line3DLagrangeP1.cpp
  Line3DLagrangeP1.hpp
  Point.hpp
  Point1DLagrangeP0.cpp
  Point1DLagrangeP0.hpp
  Point2DLagrangeP0.cpp
  Point2DLagrangeP0.hpp
  Point3DLagrangeP0.cpp
  Point3DLagrangeP0.hpp
  Quad.hpp
  Quad2DLagrangeP1.cpp
  Quad2DLagrangeP1.hpp
  Quad2DLagrangeP2.cpp
  Quad2DLagrangeP2.hpp
  Quad2DLagrangeP3.cpp
  Quad2DLagrangeP3.hpp
  Quad3DLagrangeP1.cpp
  Quad3DLagrangeP1.hpp
  SFHexaLagrangeP1.hpp
  SFHexaLagrangeP1.cpp
  SFLineLagrangeP0.cpp
  SFLineLagrangeP0.hpp
  SFLineLagrangeP1.cpp
  SFLineLagrangeP1.hpp
  SFLineLagrangeP2.cpp
  SFLineLagrangeP2.hpp
  SFLineLagrangeP3.cpp
  SFLineLagrangeP3.hpp
  SFPointLagrangeP0.cpp
  SFPointLagrangeP0.hpp
  SFQuadLagrangeP0.hpp
  SFQuadLagrangeP0.cpp
  SFQuadLagrangeP1.hpp
  SFQuadLagrangeP1.cpp
  SFQuadLagrangeP2.hpp
  SFQuadLagrangeP2.cpp
  SFQuadLagrangeP3.hpp
  SFQuadLagrangeP3.cpp
  SFTetraLagrangeP1.hpp
  SFTetraLagrangeP1.cpp
  SFTriagLagrangeP1.hpp
  SFTriagLagrangeP1.cpp
  SFTriagLagrangeP2.hpp
  SFTriagLagrangeP2.cpp
  SFTriagLagrangeP2B.hpp
  SFTriagLagrangeP2B.cpp
  SFTriagLagrangeP3.hpp
  SFTriagLagrangeP3.cpp
  Tetra.hpp
  Tetra3DLagrangeP1.cpp
  Tetra3DLagrangeP1.hpp
  Triag.hpp
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


