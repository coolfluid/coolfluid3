list( APPEND utest-fvm-shocktube_cflibs coolfluid_fvm coolfluid_mesh_generation coolfluid_mesh_gmsh coolfluid_mesh_actions )
list( APPEND utest-fvm-shocktube_files  utest-fvm-shocktube.cpp )

coolfluid_add_unittest( utest-fvm-shocktube )
