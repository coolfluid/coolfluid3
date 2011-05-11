#########################################################################################
# utest-riemannsolver

list( APPEND utest-riemannsolver_cflibs coolfluid_riemannsolvers )
list( APPEND utest-riemannsolver_files  utest-riemannsolver.cpp )

coolfluid_add_unit_test( utest-riemannsolver )

#########################################################################################
