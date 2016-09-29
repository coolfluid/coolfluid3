coolfluid_add_test( ATEST atest-actuatordisk
                    PYTHON atest-actuatordisk.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/../meshes/box.msh
                    MPI 1)
