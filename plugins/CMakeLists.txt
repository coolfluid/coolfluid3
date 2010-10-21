include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )  # add dir to include path

add_subdirectory( BlockMeshReader ) # reader of OpenFOAM block mesh format
add_subdirectory( CGAL )            # CGAL mesh generator


