// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_mpi_types_hpp
#define CF_Common_mpi_types_hpp

#include <mpi.h>

////////////////////////////////////////////////////////////////////////////////

/**
 * @file types.hpp
 * @author Tamas Banyai
 *
 * This header defines typedefs over mpi native definitions.
 */

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////////

/// communicator
typedef MPI_Comm Communicator;

/// operation (mostly for reduce and all_reduce)
typedef MPI_Op Operation;

/// datatype
typedef MPI_Datatype Datatype;

////////////////////////////////////////////////////////////////////////////////

    } // namespace mpi
  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_types_hpp
