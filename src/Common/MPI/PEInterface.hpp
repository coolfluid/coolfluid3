// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_PEInterface_hpp
#define CF_Common_PEInterface_hpp

#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/graph_communicator.hpp>
#include <boost/mpi/nonblocking.hpp>

#include "Common/WorkerStatus.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
 * @file PEInterface.hpp
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

////////////////////////////////////////////////////////////////////////////////

/**
 * Base class for the PEInterface
 * Communication with MPI always occurs over a communicator,
 * which can be created be simply default-constructing an object
 * of type boost::mpi::communicator. This communicator can then be queried
 * to determine how many processes are running (the "size" of the communicator)
 * and to give a unique number to each process, from zero to the size
 * of the communicator-1 (i.e., the "rank" of the process)
**/
class Common_API PEInterface :
    public boost::noncopyable,
    public boost::mpi::communicator 
{

public:

  /// public constructor
  PEInterface(int argc, char** args);

  /// destructor
  ~PEInterface();

  /// Return a reference to the current PE
  static PEInterface& instance();

  /// Initialise the PE
  void init(int argc=0, char** args=0);

  /// Checks if the PE is initialized
  bool is_init() const;

  /// Free the PE, careful because some mpi-s fail upon re-init after a proper finalize
  void finalize();

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

  /// Operator to boost.mpi environment, environment is noncopyable
  operator boost::mpi::environment&() { return *m_environment; }
  operator boost::mpi::environment*() { return m_environment; }

private:

  /// private constructor
  PEInterface();

  /// Current status.
  /// Default value is @c #NOT_RUNNING.
  WorkerStatus::Type m_current_status;

  /// mpi environment
  boost::mpi::environment *m_environment;

}; // PEInterface

////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PEInterface_hpp
