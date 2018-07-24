add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/actuator-2disks.msh
  COMMAND ${CF3_GMSH_EXECUTABLE} -3 ${CMAKE_CURRENT_SOURCE_DIR}/actuator-2disks.geo -o ${CMAKE_CURRENT_BINARY_DIR}/actuator-2disks.msh
DEPENDS actuator-2disks.geo)
add_custom_target(actuator-2disks-mesh ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/actuator-2disks.msh)
