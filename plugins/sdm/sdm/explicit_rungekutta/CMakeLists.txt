list( APPEND coolfluid_sdm_explicit_rungekutta_files 
  ButcherTableau.hpp
  ButcherTableau.cpp
  ExplicitRungeKutta.hpp
  ExplicitRungeKutta.cpp
  LibExplicitRungeKutta.hpp
  LibExplicitRungeKutta.cpp
  Types.hpp
  Types.cpp
)
list( APPEND coolfluid_sdm_explicit_rungekutta_cflibs coolfluid_sdm)

coolfluid_add_library( coolfluid_sdm_explicit_rungekutta )
