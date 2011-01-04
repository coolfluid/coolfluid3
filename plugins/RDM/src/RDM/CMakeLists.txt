list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  CSchemeLDA.cpp
  CSchemeLDA.hpp
  CSchemeLDAT.hpp
  ResidualDistribution.cpp
  ResidualDistribution.hpp
  RungeKutta.cpp
  RungeKutta.hpp
)

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
