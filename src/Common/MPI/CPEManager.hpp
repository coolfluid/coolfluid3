// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_MPI_CPEManager_hpp
#define CF_Common_MPI_CPEManager_hpp

#include <boost/thread/thread.hpp>

#include "Common/MPI/types.hpp"
#include "Common/MPI/CWorkerGroup.hpp"

#include "Common/Component.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////

class ListeningThread;

class CPEManager : public Component
{
public: // typedefs

  typedef boost::shared_ptr<CPEManager> Ptr;
  typedef boost::shared_ptr<const CPEManager> ConstPtr;

public: // functions

  CPEManager( const std::string & name );

  /// Destructor.
  virtual ~CPEManager();

  /// Returns the class name.
  static std::string type_name() { return "CPEManager"; }

  void spawn_group(const std::string & name, Uint nb_workers,
                   const char * command, const std::string & forward = std::string(),
                   const char * hosts = nullptr);

  void kill_group( const std::string & name );

  void kill_all();

  void wait();

  void send_to_parent( const SignalArgs & args );

  void send_to( const std::string & group, const SignalArgs & args );

  void broadcast( const SignalArgs & args );

  boost::thread & listening_thread();

  /// @name SIGNALS
  //@{

  void signal_spawn_group ( SignalArgs & args );

  void signal_kill_group ( SignalArgs & args );

  void signal_kill_all ( SignalArgs & args );

  void signal_message ( SignalArgs & args );

  void mpi_forward ( SignalArgs & args );

  void signal_exit ( SignalArgs & args );

  void new_signal ( const MPI::Intercomm &, XML::XmlDoc::Ptr );

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



}; // CPEManager

////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_MPI_CPEManager_hpp
