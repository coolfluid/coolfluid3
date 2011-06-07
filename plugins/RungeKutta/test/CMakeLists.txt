#########################################################################################
# utest-rungekutta

list( APPEND utest-rungekutta_cflibs coolfluid_rungekutta coolfluid_solver coolfluid_solver_actions coolfluid_mesh coolfluid_mesh_actions)
list( APPEND utest-rungekutta_files  utest-rungekutta.cpp )

coolfluid_add_unit_test( utest-rungekutta )

#########################################################################################
