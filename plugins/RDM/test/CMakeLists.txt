set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

##########################################################################
# unit tests

list( APPEND utest-rdm-lda_cflibs coolfluid_rdm )
list( APPEND utest-rdm-lda_files  utest-rdm-lda.cpp )

coolfluid_add_unit_test( utest-rdm-lda )

##########################################################################
# acceptance tests

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadv2d
                               SCRIPT  atest-rdm-linearadv2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadv2d-uniform
                               SCRIPT  atest-rdm-linearadv2d-uniform.cfscript )

coolfluid_add_python_test(ATEST NAME atest-rdm-rotationadv2d
                                SCRIPT atest-rdm-rotationadv2d.py)

coolfluid_add_acceptance_test( NAME    atest-rdm-burgers2d
                               SCRIPT  atest-rdm-burgers2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-euler2d-uniform-flow
                               SCRIPT  atest-rdm-euler2d-uniform-flow.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-euler3d-uniform-flow
                               SCRIPT  atest-rdm-euler3d-uniform-flow.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-euler2d-riemann
                               SCRIPT  atest-rdm-euler2d-riemann.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadv3d
                               SCRIPT  atest-rdm-linearadv3d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-sinusbump
                               SCRIPT  atest-rdm-sinusbump.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-ramp2d
                               SCRIPT  atest-rdm-ramp2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-linearadvsys2d
                               SCRIPT  atest-rdm-linearadvsys2d.cfscript )

coolfluid_add_acceptance_test( NAME    atest-rdm-lineardiff2d
                               SCRIPT  atest-rdm-lineardiff2d.cfscript )

##########################################################################
# RK atests

coolfluid_add_acceptance_test( NAME    atest-rdm-rk-lineuler2d
                               SCRIPT  atest-rdm-rk-lineuler2d.cfscript )

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
