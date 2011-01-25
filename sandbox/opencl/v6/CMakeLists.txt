include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${OPENCL_INCLUDE_DIR} )

add_executable( sandbox-opencl-matmul-v6 main.cxx matrix_mult.h matrix_mult.cpp matrix_sizes.h kernel.cl )

target_link_libraries( sandbox-opencl-matmul-v6 ${OPENCL_LIBRARIES} ${Boost_LIBRARIES} )

add_custom_command(TARGET sandbox-opencl-matmul-v6
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cl ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
