################################################################################

LIST( APPEND uTestTetraSF_cflibs SF CF_CGAL Gmsh )
LIST( APPEND uTestTetraSF_files
  uTestTetraSF.cpp )

CF_ADD_UNITTEST( uTestTetraSF )

if( DEFINED CF_CGAL_COMPILE_FLAGS )

  SET_SOURCE_FILES_PROPERTIES(uTestTetraSF.cpp PROPERTIES COMPILE_FLAGS ${CF_CGAL_COMPILE_FLAGS})

  if(CF_STRIP_CGAL_COMPILE_FLAGS)
    set(uTestTetraSF_COMPILE_FLAGS "${uTestTetraSF_COMPILE_FLAGS} ${CF_CGAL_COMPILE_FLAGS}")
  endif()

endif()

################################################################################
