LIST ( APPEND Neu_files
  NeuAPI.hpp
  NeuReader.hpp
  NeuReader.cpp
)

LIST ( APPEND Neu_cflibs Mesh )

SET  ( Neu_kernellib TRUE )

CF_ADD_LIBRARY ( Neu )
