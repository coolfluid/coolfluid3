##########################################################################

list( APPEND utest-rdm-linearadv2d_cflibs coolfluid_rdm  )
list( APPEND utest-rdm-linearadv2d_files  utest-rdm-linearadv2d.cpp )

coolfluid_add_unit_test( utest-rdm-linearadv2d )

add_custom_command(TARGET utest-rdm-linearadv2d
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-tg-p3.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-qd-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-qd-p3.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

##########################################################################

list( APPEND utest-rdm-rotationadv2d_cflibs ${CF_KERNEL_LIBS} coolfluid_rdm )
list( APPEND utest-rdm-rotationadv2d_files  utest-rdm-rotationadv2d.cpp )

coolfluid_add_unit_test( utest-rdm-rotationadv2d )

add_custom_command(TARGET utest-rdm-rotationadv2d
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

##########################################################################

list( APPEND utest-rdm-burgers2d_cflibs  coolfluid_rdm  )
list( APPEND utest-rdm-burgers2d_files  utest-rdm-burgers2d.cpp )

coolfluid_add_unit_test( utest-rdm-burgers2d )

add_custom_command(TARGET utest-rdm-burgers2d
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-tg-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-qd-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-qd-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

##########################################################################

list( APPEND utest-rdm-linearadvsys2d_cflibs  coolfluid_rdm  )
list( APPEND utest-rdm-linearadvsys2d_files  utest-rdm-linearadvsys2d.cpp )

coolfluid_add_unit_test( utest-rdm-linearadvsys2d )

##########################################################################

list( APPEND utest-rdm-euler2d_cflibs  coolfluid_rdm  )
list( APPEND utest-rdm-euler2d_files  utest-rdm-euler2d.cpp )

coolfluid_add_unit_test( utest-rdm-euler2d )

##########################################################################
  
if( NOT CF_ENABLE_GPU OR NOT CF_ENABLE_OPENCL)
  set(utest-rdm-rotationadv2dgpu_condition FALSE)
endif()
  
  list( APPEND utest-rdm-rotationadv2dgpu_cflibs ${CF_KERNEL_LIBS} coolfluid_rdm )
  list( APPEND utest-rdm-rotationadv2dgpu_files  utest-rdm-rotationadv2d-gpu.cpp )

  coolfluid_add_unit_test( utest-rdm-rotationadv2dgpu )

  add_custom_command(TARGET utest-rdm-rotationadv2d-gpu
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 #COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )
if( NOT CF_ENABLE_GPU OR NOT CF_ENABLE_OPENCL)
  set(utest-rdm-translationadv2dgpu_condition FALSE)
endif()
  
  list( APPEND utest-rdm-translationadv2dgpu_cflibs ${CF_KERNEL_LIBS} coolfluid_rdm )
  list( APPEND utest-rdm-translationadv2dgpu_files  utest-rdm-translationadv2d-gpu.cpp )

  coolfluid_add_unit_test( utest-rdm-translationadv2dgpu )

  add_custom_command(TARGET utest-rdm-rotationadv2d-gpu
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 #COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

  
##########################################################################
# acceptance tests

list( APPEND atest-rdm-burgers2d_reference ${CMAKE_CURRENT_SOURCE_DIR}/reference.msh )

coolfluid_add_acceptance_test( atest-rdm-burgers2d burgers2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-euler2d euler2d.cfscript )
