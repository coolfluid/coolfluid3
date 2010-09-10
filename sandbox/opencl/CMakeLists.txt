set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

find_package(OpenCL)

if(OPENCL_FOUND)

  add_subdirectory(v0)
  add_subdirectory(v1)

endif()
