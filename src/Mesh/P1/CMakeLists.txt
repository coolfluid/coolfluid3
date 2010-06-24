LIST ( APPEND P1_files
  P1API.hpp
  P1API.cpp
  ElemTypes.hpp
  Line1D.hpp
  Line1D.cpp
  Line2D.hpp
  Line2D.cpp
  Line3D.hpp
  Line3D.cpp
  Tetra3D.hpp
  Tetra3D.cpp
  Triag2D.hpp
  Triag2D.cpp
  Triag3D.hpp
  Triag3D.cpp
  Quad2D.hpp
  Quad2D.cpp
  Quad3D.hpp
  Quad3D.cpp
  Hexa3D.hpp
  Hexa3D.cpp
)

LIST ( APPEND P1_cflibs Mesh )

SET  ( P1_kernellib TRUE )

CF_ADD_LIBRARY ( P1 )

# ADD_SUBDIRECTORY ( uTests )    # unit tests for P1

