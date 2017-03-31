coolfluid3_add_library(TARGET coolfluid_ufem_adjointtube
  SOURCES
    AdjointTube.hpp
    AdjointTube.cpp
    DirectDifferentiation.hpp
    DirectDifferentiation.cpp
    BCSensU.hpp
    BCSensU.cpp
    BCSensUx.hpp
    BCSensUx.cpp
    BCSensUy.hpp
    BCSensUy.cpp
    BCAdjointpressurex.hpp
    BCAdjointpressurex.cpp
    BCAdjointpressurey.hpp
    BCAdjointpressurey.cpp
    RobinUt.cpp
    RobinUt.hpp
    LibUFEMAdjointTube.hpp
    LibUFEMAdjointTube.cpp

  LIBS
    coolfluid_mesh_lagrangep1
    coolfluid_mesh_lagrangep2
    coolfluid_solver_actions
    coolfluid_physics
    coolfluid_mesh
    coolfluid_ufem
  CONDITION  CF3_ENABLE_PROTO)
