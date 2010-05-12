IF (CF_HAVE_CGNS)

LIST ( APPEND CGNS3_files
  CGNSAPI.hpp
  Common.hpp 
  Common.cpp
  Reader.hpp
  Reader.cpp
)

LIST ( APPEND CGNS3_includedirs ${CGNS_INCLUDE_DIR} )
LIST ( APPEND CGNS3_libs ${CGNS_LIBRARY} )
LIST ( APPEND CGNS3_cflibs Mesh )

SET  ( CGNS3_kernellib TRUE )

CF_ADD_LIBRARY ( CGNS3 )

ENDIF (CF_HAVE_CGNS)
