
include_directories( ${CMAKE_SOURCE_DIR} ) # include base dir of project

add_subdirectory( rapidxml )
add_subdirectory( Eigen )

add_subdirectory( Common )
add_subdirectory( Mesh )
add_subdirectory( Math )
add_subdirectory( Actions )
add_subdirectory( Solver )

add_subdirectory( GUI )

add_subdirectory( Tools )
