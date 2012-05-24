# common library provides the component environment
add_subdirectory( common )

# mesh library provides the mesh database handling
add_subdirectory( mesh )

# math library provides mathematical operations
add_subdirectory( math )

# physiscs library provides access to physical models
add_subdirectory( physics )

# solver library provides access to PDE solvers
add_subdirectory( solver )

# library that exposes the python module
add_subdirectory( python )

# UI provides the user interface
add_subdirectory( ui )

# tools harbours miscellaneous tools to support coolfluid development
add_subdirectory( Tools )

# add dir with tutorial applications
add_subdirectory( tutorial )

