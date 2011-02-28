coolfluid_find_orphan_files()

# acommand-line tool to execute a batch of coolfluid commands
add_subdirectory( CommandLineInterpreter )

# acommand-line tool to execute a batch of coolfluid commands
add_subdirectory( Command )

# a library providing an interface to profiling with google perftools
add_subdirectory( GooglePerf )

# a library to help testing differences in arrays
add_subdirectory( Testing )

# a simple block mesh generator
add_subdirectory( MeshGeneration )

# a library to help testing differences in meshes
add_subdirectory( MeshDiff )

# a command-line app to manipulate meshes
add_subdirectory( MeshTransformer )
