#########################################################################################
# utest-advectiondiffusion

list( APPEND utest-advectiondiffusion_cflibs coolfluid_advectiondiffusion coolfluid_solver)
list( APPEND utest-advectiondiffusion_files  utest-advectiondiffusion.cpp )

coolfluid_add_unit_test( utest-advectiondiffusion )

#########################################################################################
# utest-euler

list( APPEND utest-euler_cflibs coolfluid_euler coolfluid_solver)
list( APPEND utest-euler_files  utest-euler.cpp )

coolfluid_add_unit_test( utest-euler )

#########################################################################################

add_subdirectory( NavierStokes )

