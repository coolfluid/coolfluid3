coolfluid_add_test( ATEST atest-actuatordisk
                    PYTHON atest-actuatordisk.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/../meshes/actuator2d.msh
                    MPI 1)

coolfluid_add_test( ATEST atest-adjoint-2disks
                    PYTHON atest-adjoint-2disks.py
                    ARGUMENTS ${CMAKE_CURRENT_BINARY_DIR}/../meshes/actuator-2disks.msh
                    MPI 4)
