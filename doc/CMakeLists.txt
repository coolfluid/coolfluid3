if( CF3_ENABLE_DOCS )

find_package(Doxygen QUIET)
coolfluid_set_package( PACKAGE Doxygen DESCRIPTION "code documentation" QUIET)

 if( DOXYGEN_FOUND )

  option( CF3_ENABLE_DOXYGEN_DOT "Enable diagrams in doxygen output" ON )
  if( CF3_ENABLE_DOXYGEN_DOT )
    if( DOXYGEN_DOT_FOUND )
      set( CF3_HAVE_DOXYGEN_DOT ON )
    endif()
  endif()

  set( DOXYFILE doxygen.config CACHE INTERNAL "Doxygen filename" )

  # we need latex for doxygen because of the formulas
  find_package(LATEX QUIET)
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

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in   ${DOXY_CONFIG}  @ONLY )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/footer.html.in
    ${CMAKE_CURRENT_SOURCE_DIR}/footer.html
  )


  add_custom_target(
    cf_doc_prerequisites
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/html/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/coolfluid.png
                                     ${CMAKE_CURRENT_BINARY_DIR}/html/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )



#  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})
  add_custom_target(doc
    COMMAND doxygen ${DOXY_CONFIG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_dependencies(doc cf_doc_prerequisites)

  # message(STATUS "Doxygen configuration ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in --> ${DOXY_CONFIG}")

 endif()

endif()
