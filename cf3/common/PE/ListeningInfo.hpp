// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_ListeningInfo_hpp
#define cf3_common_PE_ListeningInfo_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PE/Comm.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

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

} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_ListeningInfo_hpp
