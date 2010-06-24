LIST ( APPEND SF_files
  Triag2DLagrangeP1.cpp
  Triag2DLagrangeP1.hpp
)

LIST ( APPEND SF_cflibs Mesh )

SET  ( SF_kernellib TRUE )

CF_ADD_LIBRARY ( SF )


