
list( APPEND utest-cphysical-model_cflibs coolfluid_solver )
list( APPEND utest-cphysical-model_files
  uTestCPhysicalModel.cpp )

coolfluid_add_unittest( utest-cphysical-model )

#########################################################################

list( APPEND utest-forwardeuler_cflibs coolfluid_solver )
list( APPEND utest-forwardeuler_files
  utest-forwardeuler.cpp )

coolfluid_add_unittest( utest-forwardeuler )

#########################################################################

list( APPEND utest-residualdistribution_cflibs coolfluid_solver )
list( APPEND utest-residualdistribution_files
  utest-residualdistribution.cpp )

coolfluid_add_unittest( utest-residualdistribution )

##########################################################################

list( APPEND utest-scalaradvection-steady_cflibs coolfluid_solver )
list( APPEND utest-scalaradvection-steady_files
  utest-scalaradvection-steady.cpp )

coolfluid_add_unittest( utest-scalaradvection-steady )

add_custom_command(TARGET utest-scalaradvection-steady
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-scalaradvection-steady
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

##########################################################################

list( APPEND utest-scalaradvection-unsteady_cflibs coolfluid_solver )
list( APPEND utest-scalaradvection-unsteady_files
utest-scalaradvection-unsteady.cpp )

coolfluid_add_unittest( utest-scalaradvection-unsteady )

add_custom_command(TARGET utest-scalaradvection-unsteady
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

add_custom_command(TARGET utest-scalaradvection-unsteady
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

##########################################################################

list( APPEND utest-loadmesh_cflibs coolfluid_solver )
list( APPEND utest-loadmesh_files
utest-loadmesh.cpp )

coolfluid_add_unittest( utest-loadmesh )

add_custom_command(TARGET utest-loadmesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-loadmesh
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
