################################################################################

LIST ( APPEND uTestTetraSF_libs Common Mesh P1 CGAL )
LIST ( APPEND uTestTetraSF_files
  uTestTetraSF.cpp )

CF_ADD_UNITTEST ( uTestTetraSF )
SET_SOURCE_FILES_PROPERTIES(uTestTetraSF.cpp 
    PROPERTIES COMPILE_FLAGS "-DCGAL_CFG_NO_TR1_ARRAY -DCGAL_CFG_NO_TR1_TUPLE")

################################################################################