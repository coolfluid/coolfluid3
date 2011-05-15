#########################################################################################
# utest-advectiondiffusion

list( APPEND utest-advectiondiffusion_cflibs coolfluid_advectiondiffusion coolfluid_solver)
list( APPEND utest-advectiondiffusion_files  utest-advectiondiffusion.cpp )

coolfluid_add_unit_test( utest-advectiondiffusion )

#########################################################################################
