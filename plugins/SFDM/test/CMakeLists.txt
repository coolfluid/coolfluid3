list( APPEND utest-sfdm-aspects_cflibs coolfluid_sfdm coolfluid_sfdm_sf coolfluid_mesh coolfluid_mesh_actions coolfluid_mesh_gmsh)
list( APPEND utest-sfdm-aspects_files  utest-sfdm-aspects.cpp )

coolfluid_add_unit_test( utest-sfdm-aspects )



list( APPEND utest-sfdm-solver_cflibs coolfluid_sfdm coolfluid_sfdm_sf coolfluid_mesh_actions coolfluid_mesh_gmsh coolfluid_advectiondiffusion)
list( APPEND utest-sfdm-solver_files  utest-sfdm-solver.cpp )

coolfluid_add_unit_test( utest-sfdm-solver )


list( APPEND utest-sfdm-wizard_cflibs coolfluid_sfdm coolfluid_sfdm_sf coolfluid_mesh_actions coolfluid_mesh_gmsh coolfluid_advectiondiffusion)
list( APPEND utest-sfdm-wizard_files  utest-sfdm-wizard.cpp )

coolfluid_add_unit_test( utest-sfdm-wizard )

