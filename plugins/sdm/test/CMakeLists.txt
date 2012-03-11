# sdm tests

#########################################################################

coolfluid_add_test( UTEST      utest-sdm-linearadv1d
                    CPP        utest-sdm-linearadv1d.cpp
                    PLUGINS    Physics RiemannSolvers
                    LIBS       coolfluid_sdm_scalar coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_scalar
                    MPI        2 )

coolfluid_add_test( UTEST      utest-sdm-lagrange
                    CPP        utest-sdm-lagrange.cpp
                    LIBS       coolfluid_sdm )

coolfluid_add_test( UTEST      utest-sdm-convection
                    CPP        utest-sdm-convection.cpp
                    LIBS       coolfluid_sdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_scalar
                    MPI        1 )

coolfluid_add_test( UTEST      utest-sdm-transformation
                    CPP        utest-sdm-transformation.cpp
                    LIBS       coolfluid_sdm )

coolfluid_add_test( UTEST      utest-sdm-lineuler-2d
                    CPP        utest-sdm-lineuler-2d.cpp
                    LIBS       coolfluid_sdm coolfluid_sdm_lineuler)
                    
coolfluid_add_test( UTEST      utest-sdm-navierstokesmovingreference-2d
                    CPP        utest-sdm-navierstokesmovingreference-2d.cpp
                    LIBS       coolfluid_sdm coolfluid_sdm_navierstokesmovingreference)


coolfluid_add_test( ATEST      atest-sdm-euler-shocktube-1d
                    PYTHON     atest-sdm-euler-shocktube-1d.py
                    LIBS       coolfluid_sdm_navierstokes)

coolfluid_add_test( ATEST      atest-sdm-euler-shocktube-2d
                    PYTHON     atest-sdm-euler-shocktube-2d.py
                    LIBS       coolfluid_sdm_navierstokes)

coolfluid_add_test( ATEST      atest-sdm-euler-cylinder-2d
                    PYTHON     atest-sdm-euler-cylinder-2d.py
                    LIBS       coolfluid_sdm_navierstokes
                    MPI        2
                    DEPENDS    copy-resources)

coolfluid_add_test( ATEST      atest-sdm-lineuler-acousticpulse-2d
                    PYTHON     atest-sdm-lineuler-acousticpulse-2d.py
                    LIBS       coolfluid_sdm_lineuler
                    DEPENDS    copy-resources)

coolfluid_add_test( ATEST      atest-sdm-lineuler-acousticvorticityentropypulse-2d
                    PYTHON     atest-sdm-lineuler-acousticvorticityentropypulse-2d.py
                    LIBS       coolfluid_sdm_lineuler
                    DEPENDS    copy-resources)

#########################################################################
