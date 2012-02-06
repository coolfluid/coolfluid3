# list of plugins distributed with the kernel

# reader of OpenFOAM block mesh format
add_subdirectory( BlockMeshReader )

# CGAL mesh generator
add_subdirectory( CGAL )

# Riemann Solvers
add_subdirectory( RiemannSolvers )

# UFEM Finite Element solver
add_subdirectory( UFEM )

# Residual Distribution solver
add_subdirectory( RDM  )

# Residual Distribution solver
add_subdirectory( sdm  )

# Physics
add_subdirectory( Physics  )

# Plugins for the UI
add_subdirectory( ui )
