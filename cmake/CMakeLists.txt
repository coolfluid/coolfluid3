# install the cmake files
file(GLOB cmakeFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")

install( FILES ${cmakeFiles} DESTINATION ${CF_INSTALL_SHARE_DIR}/cmake )

add_subdirectory( macros )
