// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_ListeningThread_hpp
#define CF_Common_MPI_ListeningThread_hpp

////////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

// boost headers
#include <boost/asio/deadline_timer.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

// CF headers
#include "Common/MPI/ListeningInfo.hpp"
#include "Common/XML/XmlDoc.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////

  /// @brief Listener for MPI messages.

  /// This class has been designed to be used as a Boost thread. When it is
  /// running, it continuously listens to one or more  MPI intercommunicators.
  /// If there are more than one intercommunicator, all of them are listened
  /// simultanuously. The thread is started by calling @c start_listening()
  /// and stopped with @c stop_listening. A thread cannot be started if
  /// it is already running. It is not recommended to have an communicator
  /// to be listened by two threads at the same time as the behaviour can
  /// be undefined. @n

  /// Internally, the thread makes a non-blocking MPI receive on all
  /// communicators and then sleeps during some time (defined by the user).
  /// When it wakes up, it checks all the communicators and, for each of them
  /// that has new data, calls the @c new_signal signal; and then makes a new
  /// non-blocking receive. @n

  /// The fact that the thread sleeps is for performance reasons: it is to
  /// avoid the code to run useless instruction and use not needed CPU time
  /// while waiting. It is to the user to determine the best "sleeping time".
  /// The default is 100 milliseconds. @n

  /// New intercommunicators can be added on the run as well as the sleeping
  /// time can be modified. Thoes changes will be taken into account
  /// at the next iteration (after the next check).

  /// @author Quentin Gasper

  class ListeningThread
  {
  public: // data

    boost::signals2::signal< void(const MPI::Intercomm &, XML::XmlDoc::Ptr) > new_signal;

  public: // functions

//    enum WaitingAckResult
//    {
//      SUCCESS,

//      FAILURE_ON_NACK,

//      FAILURE_ON_TIMEOUT,

//      FAILURE_ALREADY_WAITING,

//      FAILURE_UNKNOWN_COMM,

//      FAILURE_NULL_COMM
//    };

    /// @brief Constructor.
    ListeningThread(Uint waitingTime = 100);

    /// @brief Adds a communicator to listen to.

    /// This method can be called during the listening.
    /// @param comm Communicator to add.
    void add_communicator(const MPI::Intercomm & comm);

    /// @brief Stops the listening.

    /// Calling this method will exit the thread execution if it is running.
    void stop_listening();

    /// @brief Starts the listening process

    /// If there is at least one communicator to listen to, the process has three
    /// main steps:
    /// @li call IRecv (non-blocking receive) on ready communicators (new ones
    /// and those that just received data)
    /// @li wait @c #m_waitingTime ms
    /// @li call check all communicators for new data
    /// When new data arrived, @c #newFrame signal is emitted. @n
    /// This process is repeated as long as @c #stop_listening() is not called.
    /// If a new communicator is added, it will be taken in account on the next
    /// iteration.
    void start_listening();

    void set_sleep_duration(Uint time);

    Uint sleep_duration() const;

  private: // function

    void check_for_data();

    void init();

  private: // data

    /// @brief Communicators

    /// The key is the communicator. The value holds information relative to
    /// the communicator.
    std::map<MPI::Intercomm, ListeningInfo> m_comms;

    /// @brief Indicates whether the thread is listening.

    /// If @c true, the thread is listening, otherwise it is not.
    bool m_listening;

    Uint m_sleep_duration;

    MPI::Intercomm m_receivingAcksComm;

    boost::mutex m_mutex;

  }; // class MPIReceiver

////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_ListeningThread_hpp
