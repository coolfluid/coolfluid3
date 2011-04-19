#######################################################################
# find MPI compiler or libraries

# try to compile an mpi program to check if compiler is already mpi
check_cxx_source_compiles(
  "#include <mpi.h>
   #include <iostream>
   int main(int argc, char* argv[])
   {
     MPI_Init(&argc, &argv); int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Finalize();
     return 0;
   }"
   CF_MPI_COMPILER_AVAILABLE )

if( CF_MPI_COMPILER_AVAILABLE )
  coolfluid_log_file( "[MPI] Already using MPI C++ compiler, no need of MPI libraries." )
  coolfluid_log_file( "     MPI CXX COMPILER   : [${CMAKE_CXX_COMPILER}]")
else()
  coolfluid_log_file( "[MPI] No MPI C++ compiler was set. Must find MPI libraries ..." )
endif()

find_package(MPI) # we always searhc for the libraries in case we need to pack them

coolfluid_log_file( "     MPI_INCLUDE_PATH   : [${MPI_INCLUDE_PATH}]")
coolfluid_log_file( "     MPI_LIBRARIES      : [${MPI_LIBRARIES}]")

###############################################################################
# check that MPI was found

if(CF_MPI_COMPILER_AVAILABLE)
  set( CF_HAVE_MPI 1 CACHE BOOL "Found MPI compiler" )
else()
  if( CF_MPI_LIBS_FOUND )
    set( CF_HAVE_MPI 1 CACHE BOOL "User enabled MPI [FOUND]" )
  else()
    message( FATAL_ERROR "[MPI] no MPI compiler or libraries were found.\n   MPI is required to compile coolfluid." )
  endif()
endif()

coolfluid_log( "CF_HAVE_MPI [${CF_HAVE_MPI}]" )

#######################################################################
# add MPI include path

# if mpi was found add it to the include path if needed
if( CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER )
  include_directories( ${MPI_INCLUDE_PATH} )
  list( APPEND CF_DEPS_LIBRARIES ${MPI_LIBRARIES} )
endif()

#######################################################################
# find mpirun

find_program( CF_MPIRUN_PROGRAM mpirun
              PATHS ${MPI_HOME}/bin $ENV{MPI_HOME}/bin
              PATH_SUFFIXES mpi/bin
              DOC "mpirun program"
              NO_DEFAULT_PATH )

find_program( CF_MPIRUN_PROGRAM mpirun
              PATH_SUFFIXES mpi/bin
              DOC "mpirun program" )

# if( NOT CF_MPIRUN_PROGRAM)
#  set(CF_MPIRUN_PROGRAM mpirun CACHE STRING "mpirun program set by default")
# endif()

coolfluid_log_file( "     CF_MPIRUN_PROGRAM : [${CF_MPIRUN_PROGRAM}]" )

mark_as_advanced( CF_HAVE_MPI CF_MPIRUN_PROGRAM )

