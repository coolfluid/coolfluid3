// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_mpi_PE_hpp
#define CF_Common_mpi_PE_hpp

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

////////////////////////////////////////////////////////////////////////////////

/**
 * @file PE.hpp
 * @author Tamas Banyai
 *
 * This header defines the the parallel environment interface.
 * The parallel interface is strongly based on Boost.MPI.
 * But since it has its issues and limitations, parts of the interface is re-implemented on COOLFluiD side.
 * Do not include any Boost.MPI header directly, rather use the headers provided in Common/MPI.
 * Dont worry, most of them are plain Boost.MPI forwards.
 */

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////////
/**
 * Base class for the PE
 * Communication with MPI always occurs over a communicator,
 * which can be created be simply default-constructing an object
 * of type boost::mpi::communicator. This communicator can then be queried
 * to determine how many processes are running (the "size" of the communicator)
 * and to give a unique number to each process, from zero to the size
 * of the communicator-1 (i.e., the "rank" of the process)
**/
class Common_API PE : public boost::noncopyable {

public:

  /// public constructor
  PE(int argc, char** args);

  /// destructor
  ~PE();

  /// Return a reference to the current PE
  static PE& instance();

  /// Operator to boost.mpi environment, environment is noncopyable
  operator Communicator() { cf_assert( is_active() ); return m_comm; }

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

  /// Collective operations - all_to_all
  template<typename T> inline T*   all_to_all(const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return mpi::all_to_all(m_comm, in_values, in_n, out_values, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           mpi::all_to_all(m_comm, in_values, out_values, stride);
  }
  template<typename T> inline T*   all_to_all(const T* in_values, const int *in_n, T* out_values, int *out_n, const int stride=1)
  {
    return mpi::all_to_all(m_comm, in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline T*   all_to_all(const T* in_values, const int *in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int stride=1)
  {
    return mpi::all_to_all(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, const std::vector<int>& in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int stride=1)
  {
           mpi::all_to_all(m_comm, in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline void all_to_all(const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int stride=1)
  {
           mpi::all_to_all(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }

  /// Collective operations - gather
  template<typename T> inline T*   gather(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return mpi::gather(m_comm, in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           mpi::gather(m_comm, in_values, out_values, root, stride);
  }
  template<typename T> inline T*   gather(const T* in_values, const int in_n, T* out_values, int *out_n, const int root, const int stride=1)
  {
    return mpi::gather(m_comm, in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline T*   gather(const T* in_values, const int in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int root, const int stride=1)
  {
    return mpi::gather(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, const int in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int root, const int stride=1)
  {
           mpi::gather(m_comm, in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline void gather(const std::vector<T>& in_values, const int in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           mpi::gather(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }

  /// Collective operations - all_gather
  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return mpi::all_gather(m_comm, in_values, in_n, out_values, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           mpi::all_gather(m_comm, in_values, out_values, stride);
  }
  template<typename T> inline void all_gather(const T& in_value, std::vector<T>& out_values)
  {
           mpi::all_gather(m_comm, in_value, out_values);           
  }  
  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, T* out_values, int *out_n, const int stride=1)
  {
    return mpi::all_gather(m_comm, in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline T*   all_gather(const T* in_values, const int in_n, const int *in_map, T* out_values, int *out_n, const int *out_map, const int stride=1)
  {
    return mpi::all_gather(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, const int in_n, std::vector<T>& out_values, std::vector<int>& out_n, const int stride=1)
  {
           mpi::all_gather(m_comm, in_values, in_n, out_values, out_n, stride);
  }
  template<typename T> inline void all_gather(const std::vector<T>& in_values, const int in_n, const std::vector<int>& in_map, std::vector<T>& out_values, std::vector<int>& out_n, const std::vector<int>& out_map, const int stride=1)
  {
           mpi::all_gather(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, stride);
  }

  /// Collective operations - scatter
  template<typename T> inline T*   scatter(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return mpi::scatter(m_comm, in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           mpi::scatter(m_comm, in_values, out_values, root, stride);
  }
  template<typename T> inline T*   scatter(const T* in_values, const int* in_n, T* out_values, int& out_n, const int root, const int stride=1)
  {
    return mpi::scatter(m_comm, in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline T*   scatter(const T* in_values, const int *in_n, const int *in_map, T* out_values, int& out_n, const int *out_map, const int root, const int stride=1)
  {
    return mpi::scatter(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, const std::vector<int>& in_n, std::vector<T>& out_values, int& out_n, const int root, const int stride=1)
  {
           mpi::scatter(m_comm, in_values, in_n, out_values, out_n, root, stride);
  }
  template<typename T> inline void scatter(const std::vector<T>& in_values, const std::vector<int>& in_n, const std::vector<int>& in_map, std::vector<T>& out_values, int& out_n, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           mpi::scatter(m_comm, in_values, in_n, in_map, out_values, out_n, out_map, root, stride);
  }

  /// Collective operations - reduce
  template<typename T, typename Op> inline T*   reduce(const Op& op, const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return mpi::reduce(m_comm, op, in_values, in_n, out_values, root, stride);
  }
  template<typename T, typename Op> inline void reduce(const Op& op, const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           mpi::reduce(m_comm, op, in_values, out_values, root, stride);
  }
  template<typename T, typename Op> inline T*   reduce(const Op& op, const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int root, const int stride=1)
  {
    return mpi::reduce(m_comm, op, in_values, in_n, in_map, out_values, out_map, root, stride);
  }
  template<typename T, typename Op> inline void reduce(const Op& op, const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           mpi::reduce(m_comm, op, in_values, in_map, out_values, out_map, root, stride);
  }

  /// Collective operations - all_reduce
  template<typename T, typename Op> inline T*   all_reduce(const Op& op, const T* in_values, const int in_n, T* out_values, const int stride=1)
  {
    return mpi::all_reduce(m_comm, op, in_values, in_n, out_values, stride);
  }
  template<typename T, typename Op> inline void all_reduce(const Op& op, const std::vector<T>& in_values, std::vector<T>& out_values, const int stride=1)
  {
           mpi::all_reduce(m_comm, op, in_values, out_values, stride);
  }
  template<typename T, typename Op> inline T*   all_reduce(const Op& op, const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int stride=1)
  {
    return mpi::all_reduce(m_comm, op, in_values, in_n, in_map, out_values, out_map, stride);
  }
  template<typename T, typename Op> inline void all_reduce(const Op& op, const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int stride=1)
  {
           mpi::all_reduce(m_comm, op, in_values, in_map, out_values, out_map, stride);
  }

  /// Collective operations - broadcast
  template<typename T> inline T*   broadcast(const T* in_values, const int in_n, T* out_values, const int root, const int stride=1)
  {
    return mpi::broadcast(m_comm, in_values, in_n, out_values, root, stride);
  }
  template<typename T> inline void broadcast(const std::vector<T>& in_values, std::vector<T>& out_values, const int root, const int stride=1)
  {
           mpi::broadcast(m_comm, in_values, out_values, root, stride);
  }
  template<typename T> inline T*   broadcast(const T* in_values, const int in_n, const int *in_map, T* out_values, const int *out_map, const int root, const int stride=1)
  {
    return mpi::broadcast(m_comm, in_values, in_n, in_map, out_values, out_map, root, stride);
  }
  template<typename T> inline void broadcast(const std::vector<T>& in_values, const std::vector<int>& in_map, std::vector<T>& out_values, const std::vector<int>& out_map, const int root, const int stride=1)
  {
           mpi::broadcast(m_comm, in_values, in_map, out_values, out_map, root, stride);
  }

private:

  /// private constructor
  PE();

  /// comm_world
  Communicator m_comm;
  /// Current status
  /// Default value is @c #NOT_RUNNING.
  WorkerStatus::Type m_current_status;

}; // PE

////////////////////////////////////////////////////////////////////////////////

} // namespace mpi
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_PE_hpp
