#########################################################################
# test physics

list( APPEND utest-solver-physics-static_cflibs coolfluid_solver )
list( APPEND utest-solver-physics-static_files  utest-solver-physics-static.cpp )

coolfluid_add_unit_test( utest-solver-physics-static )


#########################################################################
# test physics

list( APPEND utest-solver-physics_cflibs coolfluid_solver )
list( APPEND utest-solver-physics_files  utest-solver-physics.cpp )

coolfluid_add_unit_test( utest-solver-physics )

#########################################################################
# test physical model

list( APPEND utest-solver-physicalmodel_cflibs coolfluid_mesh coolfluid_solver coolfluid_mesh_generation )
list( APPEND utest-solver-physicalmodel_files  utest-solver-physicalmodel.cpp )

coolfluid_add_unit_test( utest-solver-physicalmodel )


#########################################################################
# test flowsolver

list( APPEND utest-solver-flowsolver_cflibs coolfluid_solver coolfluid_mesh coolfluid_solver_actions)
list( APPEND utest-solver-flowsolver_files  utest-solver-flowsolver.cpp )

coolfluid_add_unit_test( utest-solver-flowsolver )


########################################################################
# action tests
add_subdirectory ( Actions )
