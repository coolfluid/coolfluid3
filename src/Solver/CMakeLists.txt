CF_FIND_ORPHAN_FILES()

LIST ( APPEND coolfluid-solver_files coolfluid-solver.cpp )

LIST (APPEND coolfluid-solver_libs Common )

CF_ADD_APP (coolfluid-solver)
