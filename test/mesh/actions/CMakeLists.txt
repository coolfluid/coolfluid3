
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

coolfluid_add_test( UTEST utest-mesh-actions-fieldcreation
                    CPP   utest-mesh-actions-fieldcreation.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_lagrangep2)

coolfluid_add_test( UTEST utest-mesh-actions-rotate-translate
                    CPP   utest-mesh-actions-rotate-translate.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_lagrangep1
                  )

coolfluid_add_test( UTEST utest-mesh-actions-shortest-edge
                    PYTHON utest-mesh-actions-shortest-edge.py )
                    
coolfluid_add_test( UTEST utest-mesh-make-boundary-global
                    PYTHON utest-mesh-make-boundary-global.py
                    MPI 4)
                    
coolfluid_add_test( UTEST utest-mesh-periodic
                    PYTHON utest-mesh-periodic.py
                    MPI 4)

coolfluid_add_test( UTEST utest-mesh-wall-distance
                    PYTHON utest-mesh-wall-distance.py
                    ARGUMENTS ${CMAKE_SOURCE_DIR}/plugins/UFEM/test/meshes/ring3d-tetras.neu
                    MPI 4)
                    
