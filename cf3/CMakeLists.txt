# common library provides the component environment
add_subdirectory( common )

# mesh library provides the mesh database handling
add_subdirectory( mesh )

# math library provides mathematical operations
add_subdirectory( math )

# physiscs library provides access to physical models
add_subdirectory( Physics )

# solver library provides access to PDE solvers
add_subdirectory( Solver )

# UI provides the user interface
add_subdirectory( UI )

# tools harbours miscellaneous tools to support coolfluid development
add_subdirectory( Tools )

add_subdirectory(Python)
