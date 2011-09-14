// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_types_hpp
#define CF_Common_MPI_types_hpp

#include <mpi.h>
#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

/// @file MPI/types.hpp
/// @author Tamas Banyai
/// This header defines typedefs over mpi native definitions.

/// Macro for checking return values of any mpi calls and throws exception on error.
#define MPI_CHECK_RESULT( MPIFunc, Args )                                                     \
{                                                                                             \
  int check_result = MPIFunc Args;                                                            \
  if (check_result != MPI_SUCCESS)                                                            \
  throw CF::Common::ParallelError(FromHere(), \
    std::string("Function: ") + \
    std::string( #MPIFunc )   + \
    std::string( " did not return MPI_SUCCESS (" ) + \
    CF::Common::to_str(check_result) + \
    std::string(").") ); \
}

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

/// communicator
typedef MPI_Comm Communicator;

/// operation (mostly for reduce and all_reduce)
typedef MPI_Op Operation;

/// datatype
typedef MPI_Datatype Datatype;

////////////////////////////////////////////////////////////////////////////////

} // namespace PE
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_types_hpp
