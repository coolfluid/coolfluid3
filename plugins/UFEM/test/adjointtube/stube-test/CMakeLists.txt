coolfluid_add_test( ATEST AdjointTube
                    PYTHON AdjointTube.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/../SBuis.msh
                    MPI 1)
