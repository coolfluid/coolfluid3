#set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

if(OPENCL_FOUND)

  add_subdirectory(v0)
  add_subdirectory(v1)
  add_subdirectory(v2)
  add_subdirectory(v3)
  add_subdirectory(v4)
  add_subdirectory(v5)
  add_subdirectory(v6)
  add_subdirectory(v1v)

  
endif()
