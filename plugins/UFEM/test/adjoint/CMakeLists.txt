coolfluid_add_test( ATEST atest-actuatordisk
                    PYTHON atest-actuatordisk.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/../meshes/actuator2d.msh
                    MPI 1)

coolfluid_add_test( ATEST atest-adjoint-2disks
                    PYTHON atest-adjoint-2disks.py
                    ARGUMENTS ${CMAKE_CURRENT_BINARY_DIR}/../meshes/actuator-2disks.msh
                    MPI 4)

coolfluid_add_test( ATEST atest-actuatordiskCt-fluid
                    PYTHON atest-actuatordiskCt-fluid.py
                    ARGUMENTS ${CMAKE_CURRENT_BINARY_DIR}/../meshes/actuator-2disks-2d.msh
                    MPI 4)

coolfluid_add_test( ATEST atest-actuatordiskCt-adjoint
                    PYTHON atest-actuatordiskCt-adjoint.py
                    MPI 4)

coolfluid_add_test( ATEST atest-actuatordiskCt-dirdiff
                    PYTHON atest-actuatordiskCt-dirdiff.py
                    MPI 4)