include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${OPENCL_INCLUDE_DIR} )

add_executable( sandbox-opencl-matvecmul-v1v main.cxx matrix_matrix_mult.h matrix_mult.cpp matrix_sizes.h matrix_matrix_mult.cl matrix_vector_mult.cl  )

target_link_libraries( sandbox-opencl-matvecmul-v1v ${OPENCL_LIBRARIES} ${Boost_LIBRARIES} )

add_custom_command(TARGET sandbox-opencl-matvecmul-v1v
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/matrix_matrix_mult.cl ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/matrix_vector_mult.cl ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
