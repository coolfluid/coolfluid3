#include(RegexUtils)
#test_escape_string_as_regex()

#file(GLOB Eigen_directory_files "*")

#escape_string_as_regex(ESCAPED_CMAKE_CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

#foreach(f ${Eigen_directory_files})
#  if(NOT f MATCHES "\\.txt" AND NOT f MATCHES "${ESCAPED_CMAKE_CURRENT_SOURCE_DIR}/[.].+" AND NOT f MATCHES "${ESCAPED_CMAKE_CURRENT_SOURCE_DIR}/src")
#    list(APPEND Eigen_directory_files_to_install ${f})
#  endif()
#endforeach(f ${Eigen_directory_files})

#install(FILES
#  ${Eigen_directory_files_to_install}
#  DESTINATION ${INCLUDE_INSTALL_DIR}/Eigen COMPONENT Devel
#  )

#add_subdirectory(src)

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
${CF3_INSTALL_INCLUDE_DIR}/Eigen )