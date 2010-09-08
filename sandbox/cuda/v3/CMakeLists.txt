include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

CUDA_ADD_EXECUTABLE( matrix_mult3 main.cxx matrix_sizes.h matrix_mult.h matrix_mult.cu )

target_link_libraries( matrix_mult3 ${Boost_LIBRARIES} )
