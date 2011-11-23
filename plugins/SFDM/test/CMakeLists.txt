# SFDM tests

#########################################################################

coolfluid_add_test( UTEST      utest-sfdm-solver
                    CPP        utest-sfdm-solver.cpp
                               DummyTerm.hpp DummyTerm.cpp
                    PLUGINS    Physics RiemannSolvers
                    LIBS       coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_mesh_vtkxml coolfluid_physics_navierstokes coolfluid_physics_lineuler
                    MPI        1 )

coolfluid_add_test( UTEST      utest-sfdm-linearadv1d
                    CPP        utest-sfdm-linearadv1d.cpp
                    PLUGINS    Physics RiemannSolvers
                    LIBS       coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_scalar
                    MPI        2 )

coolfluid_add_test( UTEST      utest-sfdm-lagrange
                    CPP        utest-sfdm-lagrange.cpp
                    LIBS       coolfluid_sfdm )

coolfluid_add_test( UTEST      utest-sfdm-convection
                    CPP        utest-sfdm-convection.cpp
                    LIBS       coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_scalar
                    MPI        1 )

coolfluid_add_test( UTEST      utest-sfdm-transformation
                    CPP        utest-sfdm-transformation.cpp
                    LIBS       coolfluid_sfdm )

#########################################################################

coolfluid_add_test( UTEST      utest-sfdm-wizard
                    CPP        utest-sfdm-wizard.cpp
                    CONDITION  OFF )


coolfluid_add_test( UTEST      utest-sfdm-aspects
                    CPP        utest-sfdm-aspects.cpp
                    CONDITION  OFF )

#########################################################################

# coolfluid_add_acceptance_test( NAME atest-sfdm-linear-advection
#                                SCRIPT  atest-sfdm-linear_advection.cfscript )

# coolfluid_add_acceptance_test( NAME atest-sfdm-euler1D-shocktube-P0
#                                SCRIPT  atest-sfdm-euler1D-shocktube-P0.cfscript )

# coolfluid_add_acceptance_test( NAME atest-sfdm-euler2D-shocktube-P0
#                                SCRIPT  atest-sfdm-euler2D-shocktube-P0.cfscript )

# coolfluid_add_acceptance_test( NAME atest-sfdm-euler1D-pulse
#                                SCRIPT  atest-sfdm-euler1D-pulse.cfscript )

# coolfluid_add_acceptance_test( NAME atest-sfdm-euler2D-pulse
#                                SCRIPT  atest-sfdm-euler2D-pulse.cfscript )

