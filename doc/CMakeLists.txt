if( CF_ENABLE_DOCS )

find_package(Doxygen)

if( DOXYGEN_FOUND )

  set( DOXYFILE doxygen.config.in CACHE INTERNAL "Doxygen filename" )

  # we need latex for doxygen because of the formulas
  find_package(LATEX)
  if(NOT LATEX_COMPILER)
    message(STATUS "latex command LATEX_COMPILER not found but usually required. You will probably get warnings and user interaction on doxy run.")
  endif()
  if(NOT MAKEINDEX_COMPILER)
    message(STATUS "makeindex command MAKEINDEX_COMPILER not found but usually required.")
  endif()
  if(NOT DVIPS_CONVERTER)
    message(STATUS "dvips command DVIPS_CONVERTER not found but usually required.")
  endif()
  
  set(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}")

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}   ${DOXY_CONFIG}  @ONLY )

  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})
  
  message(STATUS "Doxygen configuration ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE} --> ${DOXY_CONFIG}")

endif(DOXYGEN_FOUND)

endif(CF_ENABLE_DOCS)
