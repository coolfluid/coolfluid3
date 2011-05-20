list( APPEND coolfluid-solver_files  coolfluid-solver.cpp
   CWorker.cpp
   CWorker.hpp
   LogForwarder.cpp
   LogForwarder.hpp )

if(CF_HAVE_PTHREAD)
  list( APPEND coolfluid-solver_libs ${PTHREAD_LIBRARIES} )
endif()

list( APPEND coolfluid-solver_cflibs ${CF_KERNEL_LIBS} )

coolfluid_add_application( coolfluid-solver )

