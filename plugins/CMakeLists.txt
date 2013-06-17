# list of plugins distributed with the kernel

# reader of OpenFOAM block mesh format
add_subdirectory( BlockMeshReader )

# CGAL mesh generator
add_subdirectory( CGAL )

# Physics
add_subdirectory( Physics  )

# UFEM Finite Element solver
add_subdirectory( UFEM )

# Plugins for the UI
add_subdirectory( ui )

add_subdirectory(vtk)