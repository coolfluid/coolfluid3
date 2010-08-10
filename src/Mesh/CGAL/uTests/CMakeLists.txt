################################################################################

LIST( APPEND uTestTetraSF_cflibs SF CF_CGAL Gmsh )
LIST( APPEND uTestTetraSF_files
  uTestTetraSF.cpp )

CF_ADD_UNITTEST( uTestTetraSF )

SET_SOURCE_FILES_PROPERTIES(uTestTetraSF.cpp 
    PROPERTIES COMPILE_FLAGS "-frounding-math -DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")

IF(CF_STRIP_CGAL_COMPILE_FLAGS)
  SET(uTestTetraSF_COMPILE_FLAGS "${uTestTetraSF_COMPILE_FLAGS} -g0 -O0")
ENDIF()

################################################################################
