
coolfluid_add_test( UTEST   utest-mesh-actions-global-connectivity
                    CPP     utest-mesh-actions-global-connectivity.cpp
                    LIBS    coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1
                    MPI     2
                    DEPENDS copy_resources )

coolfluid_add_test( UTEST   utest-mesh-actions-facebuilder
                    CPP     utest-mesh-actions-facebuilder.cpp
                    LIBS    coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1
                    DEPENDS copy_resources )

coolfluid_add_test( UTEST   utest-mesh-actions-facebuilder-mpi
                    CPP     utest-mesh-actions-facebuilder-mpi.cpp
                    LIBS    coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1
                    DEPENDS copy_resources
                    MPI     2 )

coolfluid_add_test( UTEST utest-mesh-actions-interpolate
                    CPP   utest-mesh-actions-interpolate.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_lagrangep1
                    MPI   2 )

coolfluid_add_test( UTEST utest-mesh-actions-rotate-translate
                    CPP   utest-mesh-actions-rotate-translate.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_lagrangep1
                  )

coolfluid_add_test( UTEST utest-mesh-actions-shortest-edge
                    PYTHON utest-mesh-actions-shortest-edge.py )
