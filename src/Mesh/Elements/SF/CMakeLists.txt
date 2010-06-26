LIST ( APPEND SF_files
  Line1DLagrangeP1.cpp
  Line1DLagrangeP1.hpp
  Quad2DLagrangeP1.cpp
  Quad2DLagrangeP1.hpp
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
)

LIST ( APPEND SF_cflibs Mesh )

SET  ( SF_kernellib TRUE )

CF_ADD_LIBRARY ( SF )


