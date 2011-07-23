set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

##########################################################################
# acceptance tests

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadv2d
                               SCRIPT  atest-rdm-linearadv2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-rotationadv2d
                               SCRIPT  atest-rdm-rotationadv2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-burgers2d
                               SCRIPT  atest-rdm-burgers2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-euler2d
                               SCRIPT  atest-rdm-euler2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadv3d
                               SCRIPT  atest-rdm-linearadv3d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-sinusbump
                               SCRIPT  atest-rdm-sinusbump.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadvsys2d
                               SCRIPT  atest-rdm-linearadvsys2d.cfscript )

##########################################################################
# RK atests

set( RK_TEST FALSE ) # not yet working

coolfluid_add_acceptance_test( NAME    atest-rdm-rk-rotationadv2d
                               SCRIPT  atest-rdm-rk-rotationadv2d.cfscript
                               CONDITIONS RK_TEST )


##########################################################################
# GPU  acceptance tests

if(CF_ENABLE_GPU AND OPENCL_FOUND)
  set( RDM_GPU TRUE )
else()
  set( RDM_GPU FALSE )
endif()

coolfluid_add_acceptance_test( NAME    atest-rdm-euler2d-gpu
                               SCRIPT  atest-rdm-euler2d-gpu.cfscript
                               CONDITIONS RDM_GPU )

coolfluid_add_acceptance_test( NAME    atest-rdm-rotationadv2d-gpu
                               SCRIPT  atest-rdm-rotationadv2d-gpu.cfscript
                               CONDITIONS RDM_GPU )


