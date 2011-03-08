# - Find MPI
# This module looks for MPI (Message Passing Interface) support
# Sets:
# MPI_INCLUDE_DIR  = where MPI headers can be found
# MPI_LIBRARY      = the library to link against
# CF_MPI_LIBS_FOUND = set to true after finding the library
#

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
    list( APPEND CF_TP_LIBRARIES ${MPI_LIBRARIES} )
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
