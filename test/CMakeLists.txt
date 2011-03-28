coolfluid_find_orphan_files()

include_directories( ${CMAKE_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( rapidxml )
add_subdirectory( Eigen )

add_subdirectory( Common )
add_subdirectory( Math )
add_subdirectory( Mesh )
add_subdirectory( Solver )

add_subdirectory( UI )

add_subdirectory( Tools )
