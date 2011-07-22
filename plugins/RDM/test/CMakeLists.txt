set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

##########################################################################
# acceptance tests

coolfluid_add_acceptance_test( atest-rdm-linearadv2d atest-rdm-linearadv2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-rotationadv2d atest-rdm-rotationadv2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-burgers2d atest-rdm-burgers2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-euler2d atest-rdm-euler2d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-linearadv3d atest-rdm-linearadv3d.cfscript )

coolfluid_add_acceptance_test( atest-rdm-sinusbump atest-rdm-sinusbump.cfscript )

coolfluid_add_acceptance_test( atest-rdm-linearadvsys2d atest-rdm-linearadvsys2d.cfscript )

##########################################################################
# RK atests

coolfluid_add_acceptance_test( atest-rdm-rk-rotationadv2d atest-rdm-rk-rotationadv2d.cfscript )

##########################################################################
# GPU  acceptance tests

if(CF_ENABLE_GPU AND OPENCL_FOUND)
  set( RDM_GPU TRUE )
else()
  set( RDM_GPU FALSE )
endif()

coolfluid_add_acceptance_test( atest-rdm-euler2d-gpu atest-rdm-euler2d-gpu.cfscript )

coolfluid_add_acceptance_test( atest-rdm-rotationadv2d-gpu atest-rdm-rotationadv2d-gpu.cfscript )


