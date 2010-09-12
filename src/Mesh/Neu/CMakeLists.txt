list( APPEND Neu_files
  NeuAPI.hpp
  CReader.hpp
  CReader.cpp
  CWriter.hpp
  CWriter.cpp
  Shared.hpp
  Shared.cpp
)

list( APPEND Neu_cflibs Mesh )

SET  ( Neu_kernellib TRUE )

coolfluid_add_library ( Neu )
