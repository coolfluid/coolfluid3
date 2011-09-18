#########################################################################################
# utest-riemannsolver
list( APPEND utest-riemannsolver_cflibs coolfluid_riemannsolvers coolfluid_physics_navierstokes coolfluid_physics_scalar coolfluid_physics_lineuler)
list( APPEND utest-riemannsolver_files  utest-riemannsolver.cpp )

coolfluid_add_unit_test( utest-riemannsolver )

#########################################################################################
