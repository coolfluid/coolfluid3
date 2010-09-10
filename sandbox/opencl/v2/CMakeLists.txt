include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${OPENCL_INCLUDE_DIR} )

add_executable( opencl-matmul-v2 main.cxx matrix_mult.h matrix_mult.cpp matrix_sizes.h kernel.cl )

target_link_libraries( opencl-matmul-v2 ${OPENCL_LIBRARIES} ${Boost_LIBRARIES} )

add_custom_command(TARGET opencl-matmul-v2
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cl ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
