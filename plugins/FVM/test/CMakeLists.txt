list( APPEND utest-fvm-shocktube1d_cflibs coolfluid_fvm coolfluid_mesh_generation coolfluid_mesh_gmsh coolfluid_mesh_actions )
list( APPEND utest-fvm-shocktube1d_files  utest-fvm-shocktube1d.cpp )

coolfluid_add_unit_test( utest-fvm-shocktube1d )

#########################################################################################

list( APPEND utest-fvm-shocktube2d_cflibs coolfluid_fvm coolfluid_mesh_generation coolfluid_mesh_gmsh coolfluid_mesh_tecplot coolfluid_mesh_actions )
list( APPEND utest-fvm-shocktube2d_files  utest-fvm-shocktube2d.cpp )

coolfluid_add_unit_test( utest-fvm-shocktube2d )

#########################################################################################

list( APPEND utest-fvm-buildghoststates_cflibs coolfluid_fvm )
list( APPEND utest-fvm-buildghoststates_files  utest-fvm-buildghoststates.cpp )

coolfluid_add_unit_test( utest-fvm-buildghoststates )

#########################################################################################

list( APPEND utest-fvm-fluxsplitter_cflibs coolfluid_fvm )
list( APPEND utest-fvm-fluxsplitter_files  utest-fvm-fluxsplitter.cpp )

coolfluid_add_unit_test( utest-fvm-fluxsplitter )

##########################################################################
# acceptance tests

#list( APPEND atest-fvm-shocktube2d_reference ${CMAKE_CURRENT_SOURCE_DIR}/reference.msh )

coolfluid_add_acceptance_test( atest-fvm-shocktube1d atest-fvm-shocktube1d.cfscript )

coolfluid_add_acceptance_test( atest-fvm-shocktube2d atest-fvm-shocktube2d.cfscript )


