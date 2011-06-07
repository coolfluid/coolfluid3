# list of plugins distributed with the kernel

# reader of OpenFOAM block mesh format
coolfluid_define_plugin( BlockMeshReader "${CMAKE_CURRENT_SOURCE_DIR}/BlockMeshReader" )
add_subdirectory( BlockMeshReader )

# CGAL mesh generator
coolfluid_define_plugin( CGAL "${CMAKE_CURRENT_SOURCE_DIR}/CGAL" )
add_subdirectory( CGAL )

# Riemann Solvers
coolfluid_define_plugin( RiemannSolvers "${CMAKE_CURRENT_SOURCE_DIR}/RiemannSolvers" )
add_subdirectory( RiemannSolvers )

# Runge-Kutta
coolfluid_define_plugin( RungeKutta "${CMAKE_CURRENT_SOURCE_DIR}/RungeKutta" )
add_subdirectory( RungeKutta )

# Finite Volume solver
coolfluid_define_plugin( FVM "${CMAKE_CURRENT_SOURCE_DIR}/FVM" )
add_subdirectory( FVM  )

# UFEM Finite Element solver
coolfluid_define_plugin( UFEM "${CMAKE_CURRENT_SOURCE_DIR}/UFEM" )
add_subdirectory( UFEM )

# Residual Distribution solver
coolfluid_define_plugin( RDM "${CMAKE_CURRENT_SOURCE_DIR}/RDM" )
add_subdirectory( RDM  )

# Residual Distribution solver
coolfluid_define_plugin( SFDM "${CMAKE_CURRENT_SOURCE_DIR}/SFDM" )
add_subdirectory( SFDM  )

# Physics
coolfluid_define_plugin( Physics "${CMAKE_CURRENT_SOURCE_DIR}/Physics" )
add_subdirectory( Physics  )

# Plugins for the UI
add_subdirectory( UI )
