LIST ( APPEND CGNS_files
  CGNSAPI.hpp
  Reader.hpp
  Reader.cpp
)

LIST ( APPEND CGNS_cflibs Mesh )

SET  ( CGNS_kernellib TRUE )

CF_ADD_LIBRARY ( CGNS )
