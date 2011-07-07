# - Find MPI
# This module looks for MPI (Message Passing Interface) support
# Sets:
# MPI_INCLUDE_DIR  = where MPI headers can be found
# MPI_LIBRARY      = the library to link against
# CF_MPI_LIBS_FOUND = set to true after finding the library
#


#######################################################################
# find MPI via the compiler

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

###########################################################################################
# search for MPI libraries

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

# try in user defined paths first
coolfluid_add_trial_include_path( ${MPI_HOME}/include )
coolfluid_add_trial_include_path( $ENV{MPI_HOME}/include )

find_path(MPI_INCLUDE_PATH
          NAMES mpi.h
          PATH_SUFFIXES mpi mpi/include
          PATHS
          ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH
          )

# try in these paths first and then the system ones
find_path(MPI_INCLUDE_PATH
          NAMES mpi.h
          PATH_SUFFIXES mpi mpi/include
          PATHS
          /usr/local
          /usr/local/include
          /usr/include
          "$ENV{ProgramFiles}/MPICH/SDK/Include"
          "$ENV{ProgramFiles}/MPICH2/include"
          "C:/Program Files/MPICH/SDK/Include"
          )

# search for the mpi library
coolfluid_add_trial_library_path( ${MPI_HOME}/lib )
coolfluid_add_trial_library_path( $ENV{MPI_HOME}/lib )

find_library(MPI_LIBRARY
             NAMES mpich2 mpi mpich mpich.rts
             PATH_SUFFIXES mpi/lib
             PATHS ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)

find_library(MPI_LIBRARY
             NAMES mpich2 mpi mpich mpich.rts
             PATH_SUFFIXES mpi/lib
             PATHS /usr/lib /usr/local/lib
             "$ENV{ProgramFiles}/MPICH/SDK/Lib"
             "$ENV{ProgramFiles}/MPICH2/Lib"
             "C:/Program Files/MPICH/SDK/Lib" )

# search for the mpi c++ library
find_library(MPICXX_LIBRARY
             NAMES mpi++ mpi_cxx
             PATH_SUFFIXES mpi/lib
             PATHS ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)

find_library(MPICXX_LIBRARY
             NAMES mpi++ mpi_cxx
             PATH_SUFFIXES mpi/lib
             PATHS /usr/lib /usr/local/lib
             "$ENV{ProgramFiles}/MPICH/SDK/Lib"
             "$ENV{ProgramFiles}/MPICH2/Lib"
             "C:/Program Files/MPICH/SDK/Lib" )

if( MPICXX_LIBRARY )
  list( APPEND MPI_LIBRARIES ${MPICXX_LIBRARY} )
endif()
if( MPI_LIBRARY )
  list( APPEND MPI_LIBRARIES ${MPI_LIBRARY} )
endif()

if( DEFINED MPI_EXTRA_LIBRARY_NAMES )

  foreach( mpi_extra_lib ${MPI_EXTRA_LIBRARY_NAMES} )

    # try in user defined paths first
    find_library( MPI_EXTRA_LIBRARY_${mpi_extra_lib}
                  NAMES ${mpi_extra_lib}
                  PATH_SUFFIXES mpi/lib
                  PATHS ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)

    # try in these paths first and then the system ones
    find_library( MPI_EXTRA_LIBRARY_${mpi_extra_lib}
                  NAMES ${mpi_extra_lib}
                  PATHS /usr/lib /usr/local/lib
                  "$ENV{ProgramFiles}/MPICH/SDK/Lib"
                  "$ENV{ProgramFiles}/MPICH2/Lib"
                  "C:/Program Files/MPICH/SDK/Lib" )

   mark_as_advanced( MPI_EXTRA_LIBRARY_${mpi_extra_lib} )

#    CF_DEBUG_VAR ( ${mpi_extra_lib} )
#    CF_DEBUG_VAR ( MPI_EXTRA_LIBRARY_${mpi_extra_lib} )

    if( NOT MPI_EXTRA_LIBRARY_${mpi_extra_lib} )
      message( FATAL_ERROR "User defined MPI extra lib \'${mpi_extra_lib}\' NOT FOUND" )
    else()
      list( APPEND MPI_EXTRA_LIBS ${MPI_EXTRA_LIBRARY_${mpi_extra_lib}} )
      mark_as_advanced( MPI_EXTRA_LIBS )
    endif()

   endforeach()

  list( APPEND MPI_LIBRARIES ${MPI_EXTRA_LIBS} )

endif()

if( ${CF_HAVE_MPI} )
    list( APPEND CF_DEPS_LIBRARIES ${MPI_LIBRARIES} )
endif()

if( MPI_INCLUDE_PATH AND MPI_LIBRARY )
  set(CF_MPI_LIBS_FOUND 1 CACHE BOOL "Found MPI libraries")
else()
  set(CF_MPI_LIBS_FOUND 0 CACHE BOOL "Did not find MPI libraries")
endif()

mark_as_advanced(
  MPI_INCLUDE_PATH
  MPI_LIBRARY
  MPICXX_LIBRARY
  MPI_LIBRARIES
  CF_MPI_LIBS_FOUND
)

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

#######################################################################
# add MPI include path

# if mpi was found add it to the include path if needed
if( CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER )
  include_directories( ${MPI_INCLUDE_PATH} )
  list( APPEND CF_DEPS_LIBRARIES ${MPI_LIBRARIES} )
endif()

#######################################################################
# check mpi version
# we should add some check for minimum MPI version (2.0)
#file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/mpi_version.cpp
#  "#include <mpi.h>
#   #include <iostream>
#   int main(int argc, char* argv[])
#   {
#     int version = 0;
#     int subversion = 0;
#     MPI_Get_version,(&version,&subversion);
#     std::cout << version << \".\" << subversion << std::endl;
#   }
#   " )
#add_executable( mpi_version ${CMAKE_CURRENT_BINARY_DIR}/mpi_version.cpp )
#get_target_property(mpi_version_path mpi_version LOCATION)
#execute_process(
#      COMMAND ${mpi_version_path}
#      OUTPUT_VARIABLE mpi_version_output
#      ERROR_VARIABLE mpi_version_error
#)
#coolfluid_log( "mpi_version_output [${mpi_version_output}]" )
#coolfluid_log( "mpi_version_error  [${mpi_version_error}]" )

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

set( MPI_FOUND ${CF_HAVE_MPI} )

