list( APPEND coolfluid-solver_files  coolfluid-solver.cpp LogForwarder.cpp LogForwarder.hpp  )

list( APPEND coolfluid-solver_cflibs ${CF_KERNEL_LIBS} )

coolfluid_add_application( coolfluid-solver )

