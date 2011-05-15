// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_ListeningInfo_hpp
#define CF_Common_MPI_ListeningInfo_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/MPI/PE.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////

  /// @brief Holds MPI listening information

  /// @author Quentin Gasper

  struct Common_API ListeningInfo
  {
  public:

    /// @returns buffer size (256 KB)
    static Uint buffer_size() { return 262144; }

    /// @brief Received MPI frame
    char * data;

    /// @brief Communicator to listen to
    Communicator comm;

    /// @brief Request for non-blocking listening
    MPI_Request request;

    /// @brief Indicates whether the communicator is ready to do another
    /// non-blocking receive.

    /// If @c true, the communicator is ready; if @c false, it is not.
    bool ready;

    /// @brief Constructor
    ListeningInfo();

    ~ListeningInfo();

  }; // struct MPIListeningInfo

////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_ListeningInfo_hpp
