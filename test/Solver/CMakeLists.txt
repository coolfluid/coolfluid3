#########################################################################
# test physics

list( APPEND utest-solver-physics-static_cflibs coolfluid_solver )
list( APPEND utest-solver-physics-static_files  utest-solver-physics-static.cpp )

coolfluid_add_unit_test( utest-solver-physics-static )

list( APPEND utest-solver-physics-static2dynamic_cflibs coolfluid_solver )
list( APPEND utest-solver-physics-static2dynamic_files  utest-solver-physics-static2dynamic.cpp )

coolfluid_add_unit_test( utest-solver-physics-static2dynamic )

########################################################################
# action tests
add_subdirectory( Actions )
