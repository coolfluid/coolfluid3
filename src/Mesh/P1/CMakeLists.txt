LIST ( APPEND P1_files
  P1API.hpp
  P1API.cpp
  Triag2D.hpp
  Triag2D.cpp
)

LIST ( APPEND P1_cflibs Mesh )

SET  ( P1_kernellib TRUE )

CF_ADD_LIBRARY ( P1 )

# ADD_SUBDIRECTORY ( uTests )    # unit tests for P1

