#########################################################################################
# utest-riemannsolver

# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND utest-riemannsolver_condition FALSE )

list( APPEND utest-riemannsolver_cflibs coolfluid_riemannsolvers coolfluid_advectiondiffusion coolfluid_euler)
list( APPEND utest-riemannsolver_files  utest-riemannsolver.cpp )

coolfluid_add_unit_test( utest-riemannsolver )

#########################################################################################
