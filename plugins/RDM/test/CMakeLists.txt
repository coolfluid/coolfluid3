set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

##########################################################################
# unit tests

list( APPEND utest-rdm-lda_cflibs coolfluid_rdm )
list( APPEND utest-rdm-lda_files  utest-rdm-lda.cpp )

coolfluid_add_unit_test( utest-rdm-lda )

##########################################################################
# acceptance tests

coolfluid_add_test(ATEST      atest-rdm-linearadv2d
                   CFSCRIPT   atest-rdm-linearadv2d.cfscript)

coolfluid_add_test( ATEST     atest-rdm-linearadv2d-uniform
                    CFSCRIPT  atest-rdm-linearadv2d-uniform.cfscript )

coolfluid_add_test(ATEST      atest-rdm-rotationadv2d
                   PYTHON     atest-rdm-rotationadv2d.py)

coolfluid_add_test( ATEST     atest-rdm-burgers2d
                    CFSCRIPT  atest-rdm-burgers2d.cfscript )

coolfluid_add_test( ATEST     atest-rdm-euler2d-uniform-flow
                    CFSCRIPT  atest-rdm-euler2d-uniform-flow.cfscript )

coolfluid_add_test( ATEST     atest-rdm-euler3d-uniform-flow
                    CFSCRIPT  atest-rdm-euler3d-uniform-flow.cfscript )

coolfluid_add_test( ATEST     atest-rdm-euler2d-riemann
                    CFSCRIPT  atest-rdm-euler2d-riemann.cfscript )

coolfluid_add_test( ATEST     atest-rdm-linearadv3d
                    CFSCRIPT  atest-rdm-linearadv3d.cfscript )

coolfluid_add_test( ATEST     atest-rdm-sinusbump
                    CFSCRIPT  atest-rdm-sinusbump.cfscript )

coolfluid_add_test( ATEST     atest-rdm-ramp2d
                    CFSCRIPT  atest-rdm-ramp2d.cfscript )

coolfluid_add_test( ATEST     atest-rdm-linearadvsys2d
                    CFSCRIPT  atest-rdm-linearadvsys2d.cfscript )

coolfluid_add_test( ATEST     atest-rdm-lineardiff2d
                    CFSCRIPT  atest-rdm-lineardiff2d.cfscript )

##########################################################################
# RK atests

coolfluid_add_test( ATEST     atest-rdm-rk-lineuler2d
                    CFSCRIPT  atest-rdm-rk-lineuler2d.cfscript )

##########################################################################
# GPU  acceptance tests

coolfluid_add_test( ATEST     atest-rdm-euler2d-gpu
                    CFSCRIPT  atest-rdm-euler2d-gpu.cfscript
                    CONDITION CF3_ENABLE_GPU AND OPENCL_FOUND )

coolfluid_add_test( ATEST     atest-rdm-rotationadv2d-gpu
                    CFSCRIPT  atest-rdm-rotationadv2d-gpu.cfscript
                    CONDITION CF3_ENABLE_GPU AND OPENCL_FOUND )
