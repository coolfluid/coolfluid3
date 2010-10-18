list( APPEND coolfluid_sf_files
  Hexa3DLagrangeP1.cpp
  Hexa3DLagrangeP1.hpp
  LibSF.cpp
  LibSF.hpp
  Line1DLagrangeP1.cpp
  Line1DLagrangeP1.hpp
  Line2DLagrangeP1.cpp
  Line2DLagrangeP1.hpp
  Line3DLagrangeP1.cpp
  Line3DLagrangeP1.hpp
  Quad2DLagrangeP1.cpp
  Quad2DLagrangeP1.hpp
  Quad3DLagrangeP1.cpp
  Quad3DLagrangeP1.hpp
  Tetra3DLagrangeP1.cpp
  Tetra3DLagrangeP1.hpp
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
  Triag2DLagrangeP2B.cpp
  Triag2DLagrangeP2B.hpp
  Triag3DLagrangeP1.cpp
  Triag3DLagrangeP1.hpp
  Types.hpp
)

list( APPEND coolfluid_sf_cflibs coolfluid_mesh )

set( coolfluid_sf_kernellib TRUE )

coolfluid_add_library( coolfluid_sf )


