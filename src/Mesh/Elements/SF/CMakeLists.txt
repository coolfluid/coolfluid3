LIST ( APPEND SF_files
  Line1DLagrangeP1.cpp
  Line1DLagrangeP1.hpp
  Line2DLagrangeP1.cpp
  Line2DLagrangeP1.hpp
  Quad2DLagrangeP1.cpp
  Quad2DLagrangeP1.hpp
  Tetra3DLagrangeP1.cpp
  Tetra3DLagrangeP1.hpp
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
  Types.hpp
)

LIST ( APPEND SF_cflibs Mesh )

SET  ( SF_kernellib TRUE )

CF_ADD_LIBRARY ( SF )


