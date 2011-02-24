##########################################################################

list( APPEND utest-rdm-linearadv2d-lda_cflibs coolfluid_mesh_actions coolfluid_mesh_actions coolfluid_rdm  )
list( APPEND utest-rdm-linearadv2d-lda_files  utest-rdm-linearadv2d-lda.cpp )

coolfluid_add_unittest( utest-rdm-linearadv2d-lda )

add_custom_command(TARGET utest-rdm-linearadv2d-lda
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-qd-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

##########################################################################

list( APPEND utest-rdm-rotationadv2d-lda_cflibs ${CF_KERNEL_LIBS} coolfluid_rdm )
list( APPEND utest-rdm-rotationadv2d-lda_files  utest-rdm-rotationadv2d-lda.cpp )

coolfluid_add_unittest( utest-rdm-rotationadv2d-lda )

add_custom_command(TARGET utest-rdm-rotationadv2d-lda
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-tg-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rotation-qd-p1.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

##########################################################################

list( APPEND utest-rdm-burgers2d-lda_cflibs coolfluid_mesh_actions coolfluid_mesh_actions coolfluid_rdm  )
list( APPEND utest-rdm-burgers2d-lda_files  utest-rdm-burgers2d-lda.cpp )

coolfluid_add_unittest( utest-rdm-burgers2d-lda )

add_custom_command(TARGET utest-rdm-rotationadv2d-lda
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-tg-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-qd-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/square1x1-qd-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )
