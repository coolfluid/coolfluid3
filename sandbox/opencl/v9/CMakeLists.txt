include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ./ViennaCL-1.1.0/ )

add_executable( sandbox-opencl-matmul-v9 main.cxx matrix_sizes.h openCL_functions.h  )

target_link_libraries( sandbox-opencl-matmul-v9 ${OPENCL_LIBRARIES} ${Boost_LIBRARIES} )

#add_custom_command(TARGET "sandbox-opencl-matmul-v9" )
