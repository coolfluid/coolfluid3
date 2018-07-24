coolfluid_add_test( ATEST atest-adjoint-tube
                    PYTHON atest-adjoint-tube.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/../meshes/actuator2d.msh
                    MPI 1)
