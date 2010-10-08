include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )  # add dir to include path

add_subdirectory( Common )
add_subdirectory( Mesh )
add_subdirectory( Math )
add_subdirectory( Actions )
add_subdirectory( Solver )

add_subdirectory( GUI )

add_subdirectory( Tools )
add_subdirectory( MeshTransformer )
