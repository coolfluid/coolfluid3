// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_PE_hpp
#define CF_Common_MPI_PE_hpp


#include <mpi.h>

#include "Common/StringConversion.hpp"
#include "Common/WorkerStatus.hpp"

#include "Common/MPI/types.hpp"
#include "Common/MPI/all_to_all.hpp"
#include "Common/MPI/gather.hpp"
#include "Common/MPI/all_gather.hpp"
#include "Common/MPI/scatter.hpp"
#include "Common/MPI/reduce.hpp"
#include "Common/MPI/all_reduce.hpp"
#include "Common/MPI/broadcast.hpp"


/// @file PE.hpp
/// @author Tamas Banyai
///
/// This header defines the the parallel environment interface.
/// The parallel interface is strongly based on Boost.MPI.
/// But since it has its issues and limitations, the interface is re-implemented.
///
/// @todo Quentin raised parent-child mpi_comm wrapping via pe -> faking a pe class which is not singleton???


namespace CF {
namespace Common {

/// @brief Classes offering a %MPI interface for %COOLFluiD
namespace Comm {

////////////////////////////////////////////////////////////////////////////////

/// Base class for the PE
/// Communication with MPI always occurs over a communicator
///  This communicator can then be queried
/// to determine how many processes are running (the "size" of the communicator)
/// and to give a unique number to each process, from zero to the size
/// of the communicator-1 (i.e., the "rank" of the process)

class Common_API PE : public boost::noncopyable {

public:

  /// public constructor
  PE(int argc, char** args);

  /// destructor
  ~PE();

  /// Return a reference to the current PE
  static PE& instance();

  /// @returns the generic communication channel
  Communicator communicator() { cf_assert( is_active() ); return m_comm; }

  /// Returns the MPI version
  std::string version() const;

  /// Initialise the PE
  /// @post will have a valid state
  void init(int argc=0, char** args=0);
  /// Free the PE, careful because some mpi-s fail upon re-init after a proper finalize
  /// @post will have not a valid state
  void finalize();

  /// Checks if the PE is initialized ( this is not the opposite of is_finalized )
  bool is_initialized() const;
  /// Checks if the PE is finalized ( this is not the opposite of is_init )
  bool is_finalized() const;
  /// Checks if the PE is in valid state
  /// should be initialized and Communicator pointer is set
  bool is_active() const { return is_initialized() && !is_finalized() && is_not_null(m_comm); }

  /// overload the barrier function
  void barrier();

  /// Sets a barrier on a custom communicator.
  /// @param comm The communicator to set the barrier on.
  void barrier( Communicator comm );

  /// Return rank, additionally, if is_init==0.
  Uint rank() const;

  /// Return the number of processes, or 1 if is_init==0.
  Uint size() const;

  /// Sets current process status.
  /// @param status New status
  /// @todo the name WorkerStatus is inappropriate, better to name it for example ProcessStatus
  void change_status(WorkerStatus::Type status);

  /// Gives the current process status.
  /// @return Returns the current process status
  WorkerStatus::Type status();

  /// Spawns new processes by running a specific command.
  /// @param count Number of processes to spawn.
  /// @param command The command to run.
  /// @param args Array of arguments for the command. Can be null.
  /// @param hosts List of target hosts, in comma-separated format. They
  /// need to be referenced in the host file given by the
  /// @c OMPI_MCA_orte_default_hostfile environment variable. If null or
  /// empty, processes are spawned on localhost.
  /// @return Returns an intercommunicator between this universe and
  /// the one newly created.
  Communicator spawn(int count,
                          const char * command,
                          char ** args = nullptr,
                          const char * hosts = nullptr);

  /// Gets the parent COMM_WORLD of the process
  Communicator get_parent() const;

  /// @name Collective all_to_all operations
  //@{

  template<typename T> inline T*   all_to_all(const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return Comm::all_to_all(communicator(), in_values, in_n, out_values, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           Comm::all_to_all(communicator(), in_values, out_values, stride);
  }
  template<typename T> inline T*   all_to_all(const T* in_values, const int *in_n, T* out_values, int *out_n, const int stride=1)
  {
    return Comm::all_to_all(communicator(), in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline T*   all_to_all(const T* in_values, const int *in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int stride=1)
  {
    return Comm::all_to_all(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, const std::vector<int>& in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int stride=1)
  {
           Comm::all_to_all(communicator(), in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int stride=1)
  {
           Comm::all_to_all(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }

  //@}

  /// @name Collective gather operations
  //@{

  template<typename T> inline T*   gather(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return Comm::gather(communicator(), in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           Comm::gather(communicator(), in_values, out_values, root, stride);
  }
  template<typename T> inline T*   gather(const T* in_values, const int in_n, T* out_values, int *out_n, const int root, const int stride=1)
  {
    return Comm::gather(communicator(), in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline T*   gather(const T* in_values, const int in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int root, const int stride=1)
  {
    return Comm::gather(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, const int in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int root, const int stride=1)
  {
           Comm::gather(communicator(), in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, const int in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           Comm::gather(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }

  //@}

  /// @name Collective all_gather operations
  //@{

  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return Comm::all_gather(communicator(), in_values, in_n, out_values, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           Comm::all_gather(communicator(), in_values, out_values, stride);
  }
  template<typename T> inline void all_gather(const T& in_value, std::vector<T>& out_values)
  {
           Comm::all_gather(communicator(), in_value, out_values);
  }
  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, T* out_values, int *out_n, const int stride=1)
  {
    return Comm::all_gather(communicator(), in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int stride=1)
  {
    return Comm::all_gather(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, const int in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int stride=1)
  {
           Comm::all_gather(communicator(), in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, const int in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int stride=1)
  {
           Comm::all_gather(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }

  //@}

  /// @name Collective scatter operations
  //@{

  template<typename T> inline T*   scatter(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return Comm::scatter(communicator(), in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           Comm::scatter(communicator(), in_values, out_values, root, stride);
  }
  template<typename T> inline T*   scatter(const T* in_values, const int* in_n, T* out_values, int& out_n, const int root, const int stride=1)
  {
    return Comm::scatter(communicator(), in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline T*   scatter(const T* in_values, const int *in_n, const int *in_map, T* out_values, int& out_n, const int *out_map, const int root, const int stride=1)
  {
    return Comm::scatter(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, const std::vector<int>& in_n, std::vector<T>& out_values, int& out_n, const int root, const int stride=1)
  {
           Comm::scatter(communicator(), in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, int& out_n, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           Comm::scatter(communicator(), in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }

  //@}

  /// @name Collective reduce operations
  //@{

  template<typename T, typename Op> inline T*   reduce(const Op& op, const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return Comm::reduce(communicator(), op, in_values, in_n, out_values, root, stride);
  }
  template<typename T, typename Op> inline void reduce(const Op& op, const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           Comm::reduce(communicator(), op, in_values, out_values, root, stride);
  }
  template<typename T, typename Op> inline T*   reduce(const Op& op, const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int root, const int stride=1)
  {
    return Comm::reduce(communicator(), op, in_values, in_n, in_map, out_values, out_map, root, stride);
  }
  template<typename T, typename Op> inline void reduce(const Op& op, const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           Comm::reduce(communicator(), op, in_values, in_map, out_values, out_map, root, stride);
  }

  //@}

  /// @name Collective all_reduce operations
  //@{

  template<typename T, typename Op> inline T*   all_reduce(const Op& op, const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return Comm::all_reduce(communicator(), op, in_values, in_n, out_values, stride);
  }
  template<typename T, typename Op> inline void all_reduce(const Op& op, const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           Comm::all_reduce(communicator(), op, in_values, out_values, stride);
  }
  template<typename T, typename Op> inline T*   all_reduce(const Op& op, const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int stride=1)
  {
    return Comm::all_reduce(communicator(), op, in_values, in_n, in_map, out_values, out_map, stride);
  }
  template<typename T, typename Op> inline void all_reduce(const Op& op, const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int stride=1)
  {
           Comm::all_reduce(communicator(), op, in_values, in_map, out_values, out_map, stride);
  }

  //@}

  /// @name Collective broadcast operations
  //@{

  template<typename T> inline T*   broadcast(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return Comm::broadcast(communicator(), in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void broadcast(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           Comm::broadcast(communicator(), in_values, out_values, root, stride);
  }
  template<typename T> inline T*   broadcast(const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int root, const int stride=1)
  {
    return Comm::broadcast(communicator(), in_values, in_n, in_map, out_values, out_map, root, stride);
  }
  template<typename T> inline void broadcast(const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           Comm::broadcast(communicator(), in_values, in_map, out_values, out_map, root, stride);
  }

  //@}

private:

  PE(); ///< private constructor

  Communicator m_comm; ///< comm_world

  WorkerStatus::Type m_current_status; ///< Current status, default value is @c #NOT_RUNNING.

}; // PE

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Comm
} // namespace Common
} // namespace CF

#endif // CF_Common_MPI_PE_hpp
