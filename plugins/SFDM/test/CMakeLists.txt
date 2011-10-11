# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND utest-sfdm-aspects_condition FALSE )
list( APPEND utest-sfdm-wizard_condition FALSE )


list( APPEND utest-sfdm-aspects_cflibs coolfluid_sfdm coolfluid_sfdm_sf coolfluid_mesh coolfluid_mesh_actions coolfluid_mesh_gmsh coolfluid_advectiondiffusion coolfluid_euler)
list( APPEND utest-sfdm-aspects_files  utest-sfdm-aspects.cpp )

coolfluid_add_unit_test( utest-sfdm-aspects )

list( APPEND utest-sfdm-solver_cflibs coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_mesh_vtkxml coolfluid_physics_navierstokes coolfluid_physics_lineuler)
list( APPEND utest-sfdm-solver_files
    utest-sfdm-solver.cpp
    DummyTerm.hpp
    DummyTerm.cpp
    )

coolfluid_add_unit_test( utest-sfdm-solver )


list( APPEND utest-sfdm-linearadv1d_cflibs coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_scalar)
list( APPEND utest-sfdm-linearadv1d_files  utest-sfdm-linearadv1d.cpp  )
set( utest-sfdm-linearadv1d_mpi_test TRUE )
coolfluid_add_unit_test( utest-sfdm-linearadv1d )


list( APPEND utest-sfdm-wizard_cflibs coolfluid_sfdm coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_physics_navierstokes)
list( APPEND utest-sfdm-wizard_files  utest-sfdm-wizard.cpp )

coolfluid_add_unit_test( utest-sfdm-wizard )


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

