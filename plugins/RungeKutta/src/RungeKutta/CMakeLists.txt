list( APPEND coolfluid_rungekutta_files
  LibRungeKutta.hpp
  LibRungeKutta.cpp
  RK.hpp
  RK.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_rungekutta_cflibs coolfluid_math coolfluid_solver coolfluid_solver_actions)

coolfluid_add_library( coolfluid_rungekutta )
