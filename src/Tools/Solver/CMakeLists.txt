list( APPEND coolfluid-solver_files  coolfluid-solver.cpp
   CWorker.cpp
   CWorker.hpp
   LogForwarder.cpp
   LogForwarder.hpp )

list( APPEND coolfluid-solver_cflibs ${PTHREAD_LIBRARIES} ${CF_KERNEL_LIBS} )

coolfluid_add_application( coolfluid-solver )

