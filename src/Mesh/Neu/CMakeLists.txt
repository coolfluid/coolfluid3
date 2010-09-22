list( APPEND Neu_files
  CReader.hpp
  CReader.cpp
  CWriter.hpp
  CWriter.cpp
  LibNeu.cpp
  LibNeu.hpp
  Shared.hpp
  Shared.cpp
)

list( APPEND Neu_cflibs Mesh )

set( Neu_kernellib TRUE )

coolfluid_add_library ( Neu )
