#########################################################################################
# utest-riemannsolver

list( APPEND utest-riemannsolver_cflibs coolfluid_riemannsolvers coolfluid_advectiondiffusion coolfluid_euler)
list( APPEND utest-riemannsolver_files  utest-riemannsolver.cpp )

coolfluid_add_unit_test( utest-riemannsolver )

#########################################################################################
