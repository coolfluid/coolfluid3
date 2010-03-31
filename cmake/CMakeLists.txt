# install the cmake files
file (GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
install( FILES ${cmakeFiles} DESTINATION cmake )

add_subdirectory ( macros )
