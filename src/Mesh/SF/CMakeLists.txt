list( APPEND SF_files
  Hexa3DLagrangeP1.cpp
  Hexa3DLagrangeP1.hpp
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
  SFLib.hpp
  Tetra3DLagrangeP1.cpp
  Tetra3DLagrangeP1.hpp
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
  Triag3DLagrangeP1.cpp
  Triag3DLagrangeP1.hpp
  Types.hpp
)

list( APPEND SF_cflibs Mesh )

set( SF_kernellib TRUE )

coolfluid_add_library ( SF )


