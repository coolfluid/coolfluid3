#########################################################################
# tests the rdm component

list( APPEND utest-rdm_cflibs coolfluid_rdm  )
list( APPEND utest-rdm_files  utest-rdm.cpp )

coolfluid_add_unittest( utest-rdm )

##########################################################################

list( APPEND utest-rdm-scalaradvection-steady_cflibs coolfluid_rdm coolfluid_mesh_neutral )
list( APPEND utest-rdm-scalaradvection-steady_files  utest-rdm-scalaradvection-steady.cpp )

coolfluid_add_unittest( utest-rdm-scalaradvection-steady )

add_custom_command(TARGET utest-rdm-scalaradvection-steady
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

##########################################################################

list( APPEND utest-rdm-scalaradvection-unsteady_cflibs coolfluid_rdm coolfluid_mesh_neutral )
list( APPEND utest-rdm-scalaradvection-unsteady_files  utest-rdm-scalaradvection-unsteady.cpp )

coolfluid_add_unittest( utest-rdm-scalaradvection-unsteady )

add_custom_command(TARGET utest-rdm-scalaradvection-unsteady
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

#########################################################################
# test runge kutta

list( APPEND utest-rdm-rungekutta_cflibs coolfluid_solver coolfluid_rdm coolfluid_mesh_neutral )
list( APPEND utest-rdm-rungekutta_files  utest-rdm-rungekutta.cpp )

coolfluid_add_unittest( utest-rdm-rungekutta )
