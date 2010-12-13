
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

list( APPEND utest-scalaradvection_cflibs coolfluid_solver )
list( APPEND utest-scalaradvection_files
  utest-scalaradvection.cpp )

coolfluid_add_unittest( utest-scalaradvection )

add_custom_command(TARGET utest-scalaradvection
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-scalaradvection
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
