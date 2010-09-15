// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_server_MPIListeningInfo_h
#define CF_server_MPIListeningInfo_h

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {
      
////////////////////////////////////////////////////////////////////////////
      
  /// @brief Hold MPI listening information
  
  /// @author Quentin Gasper
  
  struct MPIListeningInfo
  {
  public:
    
    /// @returns buffer size
    static Uint buffer_size() { return 65536; }
    
    /// @brief Counter
    /// @todo this attribute should be removed
    int m_counter;
    
    /// @brief Indicates whether the listening is finished for this communicator
    /// @todo this attribute should be removed
    bool m_finished;
    
    /// @brief Number of processes in the communicator
    /// @todo this attribute should be removed
    int m_processCount;
    
    /// @brief Received MPI frame
    char * m_data;
    
    /// @brief Communicator to listen to
    MPI::Intercomm m_comm;
    
    /// @brief Request for non-blocking listening
    MPI::Request m_request;
    
    /// @brief Indicates whether the communicator is ready to do another
    /// non-blocking receive.
    
    /// If @c true, the communicator is ready; if @c false, it is not.
    bool m_ready;
    
    /// @brief Constructor
    MPIListeningInfo();
    
    /// @brief Sets intercommunicator to listen to.
    
    /// Structure data are reinitialized.
    /// @param comm Intercommunicator
    void setComm(MPI::Intercomm comm);
    
    /// @brief Increment the counter and update @c #finished attribute.
    /// @todo this method should be removed
    void incCounter();
    
  }; // struct MPIListeningInfo

////////////////////////////////////////////////////////////////////////////
  
} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_server_MPIListeningInfo_h
