list( APPEND coolfluid-solver_files  coolfluid-solver.cpp
   CWorker.cpp
   CWorker.hpp
   Notifier.hpp
   Notifier.cpp
   LogForwarder.cpp
   LogForwarder.hpp )

if(CF3_HAVE_PTHREAD)
  list( APPEND coolfluid-solver_libs ${PTHREAD_LIBRARIES} )
endif()

if( ParaView_FOUND )
  add_definitions( -DCF3_HAVE_PARAVIEW )
  include_directories( ${PARAVIEW_INCLUDE_DIRS} )
  list( APPEND coolfluid-solver_libs coolfluid_ui_paraview )
endif()


list( APPEND coolfluid-solver_cflibs ${CF3_KERNEL_LIBS} )

coolfluid_add_application( coolfluid-solver )

