set( RDM_TEST_RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "dir for running RDM tests" )

##########################################################################
# unit tests

coolfluid_add_test( UTEST  utest-rdm-lda
                    CPP    utest-rdm-lda.cpp
                    LIBS   coolfluid_rdm )

##########################################################################
# acceptance tests

coolfluid_add_test( ATEST     atest-rdm-linearadv2d
                    PYTHON    atest-rdm-linearadv2d.py)

coolfluid_add_test( ATEST     atest-rdm-linearadv2d-uniform
                    PYTHON    atest-rdm-linearadv2d-uniform.py )

coolfluid_add_test( ATEST     atest-rdm-rotationadv2d
                    PYTHON    atest-rdm-rotationadv2d.py)

coolfluid_add_test( ATEST     atest-rdm-burgers2d
                    PYTHON    atest-rdm-burgers2d.py )

coolfluid_add_test( ATEST     atest-rdm-euler2d-uniform-flow
                    PYTHON    atest-rdm-euler2d-uniform-flow.py )

coolfluid_add_test( ATEST     atest-rdm-euler3d-uniform-flow
                    PYTHON    atest-rdm-euler3d-uniform-flow.py )

coolfluid_add_test( ATEST     atest-rdm-euler2d-riemann
                    PYTHON    atest-rdm-euler2d-riemann.py )

coolfluid_add_test( ATEST     atest-rdm-linearadv3d
                    PYTHON    atest-rdm-linearadv3d.py )

coolfluid_add_test( ATEST     atest-rdm-sinusbump
                    PYTHON    atest-rdm-sinusbump.py
                    CONDITION FALSE) # Disabled since nobody maintains this atest

coolfluid_add_test( ATEST     atest-rdm-ramp2d
                    PYTHON    atest-rdm-ramp2d.py )

coolfluid_add_test( ATEST     atest-rdm-linearadvsys2d
                    PYTHON    atest-rdm-linearadvsys2d.py )

coolfluid_add_test( ATEST     atest-rdm-lineardiff2d
                    PYTHON    atest-rdm-lineardiff2d.py )

##########################################################################
# RK atests

coolfluid_add_test( ATEST     atest-rdm-rk-lineuler2d
                    PYTHON    atest-rdm-rk-lineuler2d.py )

##########################################################################
# GPU  acceptance tests

coolfluid_add_test( ATEST     atest-rdm-euler2d-gpu
                    CFSCRIPT  atest-rdm-euler2d-gpu.cfscript
                    CONDITION CF3_ENABLE_GPU AND OPENCL_FOUND )

coolfluid_add_test( ATEST     atest-rdm-rotationadv2d-gpu
                    CFSCRIPT  atest-rdm-rotationadv2d-gpu.cfscript
                    CONDITION CF3_ENABLE_GPU AND OPENCL_FOUND )
