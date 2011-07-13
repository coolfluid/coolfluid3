#########################################################################
# test physics

list( APPEND utest-solver-physics-static_cflibs coolfluid_solver )
list( APPEND utest-solver-physics-static_files  utest-solver-physics-static.cpp )

coolfluid_add_unit_test( utest-solver-physics-static )

list( APPEND utest-solver-physics-static2dynamic_cflibs coolfluid_solver )
list( APPEND utest-solver-physics-static2dynamic_files  utest-solver-physics-static2dynamic.cpp )

coolfluid_add_unit_test( utest-solver-physics-static2dynamic )

#########################################################################
# test flowsolver

list( APPEND utest-solver-flowsolver_cflibs coolfluid_solver coolfluid_mesh coolfluid_solver_actions)
list( APPEND utest-solver-flowsolver_files  utest-solver-flowsolver.cpp )

coolfluid_add_unit_test( utest-solver-flowsolver )


########################################################################
# action tests
add_subdirectory( Actions )
