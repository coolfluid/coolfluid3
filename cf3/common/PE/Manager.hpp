// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_Manager_hpp
#define cf3_common_PE_Manager_hpp

#include "common/PE/types.hpp"
#include "common/PE/WorkerGroup.hpp"

#include "common/Component.hpp"

namespace boost { class thread; }

namespace cf3 {
namespace common {

class NotificationQueue;
namespace XML { class XmlDoc; }

namespace PE {

/////////////////////////////////////////////////////////////////////////////////////////////

class ListeningThread;

class Manager : public Component
{
public: // functions

  Manager( const std::string & name );

  /// Destructor.
  virtual ~Manager();

  /// Returns the class name.
  static std::string type_name() { return "Manager"; }

  void spawn_group(const std::string & name, Uint nb_workers,
                   const char * command, const std::string & forward = std::string(),
                   const char * hosts = nullptr);

  void kill_group( const std::string & name );

  void kill_all();

  void wait();

  void send_to_parent( const SignalArgs & args );

  void send_to( const std::string & group, const SignalArgs & args );

  void broadcast( const SignalArgs & args );

  boost::thread * listening_thread();

  common::NotificationQueue * notification_queue() { return m_queue; }

  const common::NotificationQueue * notification_queue() const { return m_queue; }

  void new_event( SignalArgs &args );

  /// @name SIGNALS
  //@{

  void signal_spawn_group ( SignalArgs & args );

  void signal_kill_group ( SignalArgs & args );

  void signal_kill_all ( SignalArgs & args );

  void signal_message ( SignalArgs & args );

  void mpi_forward ( SignalArgs & args );

  void signal_exit ( SignalArgs & args );

  void new_signal ( const ::MPI::Intercomm &, boost::shared_ptr<XML::XmlDoc> );

  //@} END SIGNALS

  /// @name SIGNAL SIGNATURES
  //@{

  void signature_spawn_group ( SignalArgs & args );

  void signature_kill_group ( SignalArgs & args );

  //@} END SIGNAL SIGNATURES

private: // functions

  void send_to( Communicator comm, const SignalArgs & args );

private:

  std::map<std::string, Communicator> m_groups;

  ListeningThread * m_listener;

  common::NotificationQueue * m_queue;

}; // Manager

/////////////////////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

#endif // cf3_common_PE_Manager_hpp
