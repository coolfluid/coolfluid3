################################################################################

LIST( APPEND uTestTetraSF_cflibs SF ICGAL Gmsh )
LIST( APPEND uTestTetraSF_files
  uTestTetraSF.cpp )

coolfluid_add_unittest( uTestTetraSF )

if( DEFINED ICGAL_COMPILE_FLAGS )

if( CMAKE_COMPILER_IS_GNUCC )

  set_source_files_properties(uTestTetraSF.cpp PROPERTIES COMPILE_FLAGS ${ICGAL_COMPILE_FLAGS})

  if(CF_STRIP_CGAL_COMPILE_FLAGS)
    set(uTestTetraSF_COMPILE_FLAGS "${uTestTetraSF_COMPILE_FLAGS} ${ICGAL_COMPILE_FLAGS}")
  endif()
endif( CMAKE_COMPILER_IS_GNUCC )

endif()

################################################################################
