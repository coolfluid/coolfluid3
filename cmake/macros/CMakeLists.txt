# install the cmake files
file(GLOB macroFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake")
install( FILES ${macroFiles} DESTINATION ${CF_INSTALL_SHARE_DIR}/cmake )
