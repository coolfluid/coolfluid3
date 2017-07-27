coolfluid3_add_library(TARGET coolfluid_ufem_adjointtube
  SOURCES
    AdjointSensitivity.hpp
    AdjointSensitivity.cpp
    AdjointTube.hpp
    AdjointTube.cpp
    DirectDifferentiation.hpp
    DirectDifferentiation.cpp
    DirectDifferentiationSensitivity.hpp
    DirectDifferentiationSensitivity.cpp
    BCSensP.hpp
    BCSensP.cpp
    BCSensU.hpp
    BCSensU.cpp
    BCAdjointpressurex.hpp
    BCAdjointpressurex.cpp
    BCAdjointpressurey.hpp
    BCAdjointpressurey.cpp
    DirDiffSensP.hpp
    DirDiffSensP.cpp
    RobinUt.cpp
    RobinUt.hpp
    RobinSensU.cpp
    RobinSensU.hpp
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
