// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_types_hpp
#define cf3_common_PE_types_hpp

#include <mpi.h>
#include "common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

/// @file MPI/types.hpp
/// @author Tamas Banyai
/// This header defines typedefs over mpi native definitions.

/// Macro for checking return values of any mpi calls and throws exception on error.
#define MPI_CHECK_RESULT( MPIFunc, Args )                                                     \
{                                                                                             \
  int check_result = MPIFunc Args;                                                            \
  if (check_result != MPI_SUCCESS)                                                            \
  throw cf3::common::ParallelError(FromHere(), \
    std::string("Function: ") + \
    std::string( #MPIFunc )   + \
    std::string( " did not return MPI_SUCCESS (" ) + \
    cf3::common::to_str(check_result) + \
    std::string(").") ); \
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
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
} // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_types_hpp
