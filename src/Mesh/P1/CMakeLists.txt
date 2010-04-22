LIST ( APPEND P1_files
  P1API.hpp
  P1API.cpp
  Triag2D.hpp
  Triag2D.cpp
)

LIST ( APPEND P1_cflibs Mesh )

CF_ADD_LIBRARY ( P1 )
