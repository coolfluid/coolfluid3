list( APPEND utest-fvm-shocktube_cflibs coolfluid_fvm coolfluid_mesh_generation coolfluid_mesh_gmsh coolfluid_mesh_actions )
list( APPEND utest-fvm-shocktube_files  utest-fvm-shocktube.cpp )

coolfluid_add_unit_test( utest-fvm-shocktube )

#########################################################################################

list( APPEND utest-fvm-fluxsplitter_cflibs coolfluid_fvm )
list( APPEND utest-fvm-fluxsplitter_files  utest-fvm-fluxsplitter.cpp )

coolfluid_add_unit_test( utest-fvm-fluxsplitter )
