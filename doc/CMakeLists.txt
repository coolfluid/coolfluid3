if( CF_ENABLE_DOCS )

find_package(Doxygen)

 if( DOXYGEN_FOUND )

  set( DOXYFILE doxygen.config CACHE INTERNAL "Doxygen filename" )

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

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in   ${DOXY_CONFIG}  @ONLY )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/footer.html.in
    ${CMAKE_CURRENT_SOURCE_DIR}/footer.html
  )
  
  
  add_custom_target(
    doc-cf-prerequisites
    ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/html/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/coolfluid.png
                                     ${CMAKE_CURRENT_BINARY_DIR}/html/
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  
  
  
#  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})
  add_custom_target(doc ALL
    COMMAND doxygen ${DOXY_CONFIG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_dependencies(doc doc-cf-prerequisites)
  
  # message(STATUS "Doxygen configuration ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in --> ${DOXY_CONFIG}")

 endif()

endif()
