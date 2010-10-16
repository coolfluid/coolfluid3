include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${OPENCL_INCLUDE_DIR} )

add_executable( sandbox-opencl-vadd main.c example.cl )

target_link_libraries( sandbox-opencl-vadd ${OPENCL_LIBRARIES} )

add_custom_command(TARGET sandbox-opencl-vadd
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/example.cl ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
