// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_ListeningThread_hpp
#define cf3_common_PE_ListeningThread_hpp

////////////////////////////////////////////////////////////////////////////////

// boost headers
//#include <boost/asio/deadline_timer.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

// CF headers
#include "common/XML/XmlDoc.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////

  struct ListeningInfo;

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
  /// The default is 10 milliseconds. @n

  /// However, be careful when defining this sleeping time. The biggest it is,
  /// the longuest is the response time for requests. If the sleeping time is too
  /// small, the CPU usage might be too high if no data arrives during a long time. @n

  /// New intercommunicators can be added on the run as well as the sleeping
  /// time can be modified. Thoes changes will be taken into account
  /// at the next iteration (after the next check).

  /// @author Quentin Gasper

  class Common_API ListeningThread
  {
  public: // data

    boost::signals2::signal< void(const Communicator &, boost::shared_ptr<XML::XmlDoc>) > new_signal;

  public: // functions

    /// @brief Constructor.
    ListeningThread(Uint waitingTime = 10);

    /// @brief Adds a communicator to listen to.

    /// This method can be called during the listening.
    /// @param comm Communicator to add.
    void add_communicator(Communicator comm);

    void remove_comunicator(Communicator comm);

    /// @brief Stops the listening.

    /// Calling this method will exit the thread execution if it is running.
    void stop_listening();

    /// @brief Starts the listening process

    /// If there is at least one communicator to listen to, the process has three
    /// main steps:
    /// @li call Irecv (non-blocking receive) on ready communicators (new ones
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

    boost::thread & thread();

  private: // function

    void run();

    void check_for_data();

    void init();

  private: // data

    /// @brief Communicators

    /// The key is the communicator. The value holds information relative to
    /// the communicator.
    std::map<Communicator, ListeningInfo*> m_comms;

    /// @brief Indicates whether the thread is listening.

    /// If @c true, the thread is listening, otherwise it is not.
    bool m_listening;

    Uint m_sleep_duration;

    Communicator m_receivingAcksComm;

    boost::mutex m_mutex;

    boost::thread m_thread;

  }; // class MPIReceiver

////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_ListeningThread_hpp
