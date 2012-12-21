#######################################################################
# find MPI compiler or libraries

set( MPI_C_FIND_QUIETLY       ON )
set( MPI_CXX_FIND_QUIETLY     ON )
set( MPI_Fortran_FIND_QUIETLY ON )

find_package( MPI ) # Use the standard CMake FindMPI

coolfluid_log_file( "     MPI_C_FOUND           : [${MPI_C_FOUND}]")
coolfluid_log_file( "     MPI_C_COMPILER        : [${MPI_C_COMPILER}]")
coolfluid_log_file( "     MPI_C_INCLUDE_PATH    : [${MPI_C_INCLUDE_PATH}]")
coolfluid_log_file( "     MPI_C_LIBRARIES       : [${MPI_C_LIBRARIES}]")

coolfluid_log_file( "     MPI_CXX_FOUND         : [${MPI_CXX_FOUND}]")
coolfluid_log_file( "     MPI_CXX_COMPILER      : [${MPI_CXX_COMPILER}]")
coolfluid_log_file( "     MPI_CXX_INCLUDE_PATH  : [${MPI_INCLUDE_PATH}]")
coolfluid_log_file( "     MPI_CXX_LIBRARIES     : [${MPI_LIBRARIES}]")

coolfluid_log_file( "     MPIEXEC               : [${MPIEXEC}]")

#######################################################################
# add MPI include path

# FindMPI has new behavior 
# Lets add MPI_CXX_INCLUDE_PATH, iff regular compiler is not MPI compiler

if( MPI_CXX_FOUND )

    if( CMAKE_CXX_COMPILER STREQUAL MPI_CXX_COMPILER AND NOT CF3_MPI_USE_HEADERS )
      coolfluid_log_file( "MPI headers *NOT* explicitly added: ${MPI_CXX_INCLUDE_PATH}")
    else()
      include_directories( ${MPI_CXX_INCLUDE_PATH} )
      list( APPEND CF3_DEPS_LIBRARIES ${MPI_CXX_LIBRARIES} )
      coolfluid_log_file( "MPI headers explicitly added: ${MPI_CXX_INCLUDE_PATH}")
    endif()

endif()

coolfluid_set_package( PACKAGE MPI DESCRIPTION "MPI communication"
                       TYPE REQUIRED
                       QUIET
                       VARS MPI_CXX_FOUND
                       )

### FAIL - FTM, MPI is required 
if( NOT MPI_CXX_FOUND )
  message( FATAL_ERROR "[MPI] no MPI compiler or libraries were found.\n   MPI is required to compile coolfluid." )
endif()

#coolfluid_debug_var( MPI_C_COMPILER )
#coolfluid_debug_var( MPI_C_NO_INTERROGATE )
#coolfluid_debug_var( MPI_C_INCLUDE_PATH )
#coolfluid_debug_var( MPI_C_LIBRARIES )

#coolfluid_debug_var( MPI_CXX_COMPILER )
#coolfluid_debug_var( MPI_CXX_NO_INTERROGATE )
#coolfluid_debug_var( MPI_CXX_INCLUDE_PATH )
#coolfluid_debug_var( MPI_CXX_LIBRARIES )

### DEPRECATED

#coolfluid_debug_var( MPI_COMPILER )
#coolfluid_debug_var( MPI_CXX_NO_INTERROGATE )
#coolfluid_debug_var( MPI_INCLUDE_PATH )
#coolfluid_debug_var( MPI_LIBRARIES )
