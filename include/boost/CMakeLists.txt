file( GLOB_RECURSE BOOST_QUADRATURE_FIND_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach( efound ${BOOST_QUADRATURE_FIND_FILES} )
  if( IS_DIRECTORY "${efound}" )
  else()
    list( APPEND BOOST_QUADRATURE_FIND_FILES ${efound} )
  endif()
endforeach()

source_group( Headers FILES ${BOOST_QUADRATURE_FIND_FILES} )

# install boost headers
install( FILES ${BOOST_QUADRATURE_FIND_FILES}
DESTINATION
${CF_INSTALL_INCLUDE_DIR}/boost )
