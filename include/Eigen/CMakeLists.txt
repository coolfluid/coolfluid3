file( GLOB_RECURSE EIGEN_FIND_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach( efound ${EIGEN_FIND_FILES} )

  if( IS_DIRECTORY "${efound}" )
  else()
      list( APPEND EIGEN_FOUND_FILES ${efound} )
  endif()

endforeach()

source_group( Headers FILES ${EIGEN_FOUND_FILES} )

# install Eigen headers
install( FILES ${EIGEN_FOUND_FILES}
         DESTINATION
         ${CF_INSTALL_INCLUDE_DIR}/Eigen )

