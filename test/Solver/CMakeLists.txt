#########################################################################
# test physical model

list( APPEND utest-solver-physics_cflibs coolfluid_solver )
list( APPEND utest-solver-physics_files  utest-solver-physics.cpp )

coolfluid_add_unit_test( utest-solver-physics )

#########################################################################
# test physical model

list( APPEND utest-solver-physicalmodel_cflibs coolfluid_solver )
list( APPEND utest-solver-physicalmodel_files  utest-solver-physicalmodel.cpp )

coolfluid_add_unit_test( utest-solver-physicalmodel )


########################################################################
# action tests
add_subdirectory ( Actions )