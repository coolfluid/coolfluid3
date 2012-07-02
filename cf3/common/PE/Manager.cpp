// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/thread/thread.hpp>
#include <boost/assign/std/vector.hpp> // for 'operator+=()'

#include <coolfluid-paths.hpp>

#include "rapidxml/rapidxml.hpp"

#include "common/BasicExceptions.hpp"
#include "common/BoostFilesystem.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
#include "common/NotificationQueue.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Signal.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/ListeningThread.hpp"

#include "common/PE/Manager.hpp"
#include "common/PE/WorkerGroup.hpp"

#include "common/Log.hpp"

using namespace boost::assign; // bring 'operator+=()' into scope
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Manager, Component, LibCommon > Manager_Builder;

////////////////////////////////////////////////////////////////////////////

Manager::Manager ( const std::string & name )
  : Component(name)
{
  m_listener = new ListeningThread();
  m_queue = new NotificationQueue();

  if( Comm::instance().get_parent() != MPI_COMM_NULL )
  {
    m_groups["MPI_Parent"] = Comm::instance().get_parent();
    m_listener->add_communicator( Comm::instance().get_parent() );
    m_listener->start_listening();
  }

  regist_signal( "spawn_group" )
      .description("Creates a new group of workers")
      .pretty_name("Spawn new group")
      .connect( boost::bind(&Manager::signal_spawn_group, this, _1) );

  regist_signal( "kill_group" )
      .description("Kills a group of workers")
      .pretty_name("Kill group")
      .connect( boost::bind(&Manager::signal_kill_group, this, _1) );

  regist_signal( "kill_all" )
      .description("Kills all groups of workers")
      .hidden(true)
      .pretty_name("Kill all groups")
      .connect( boost::bind(&Manager::signal_kill_all, this, _1) );

  regist_signal("exit")
      .connect( boost::bind(&Manager::signal_exit, this, _1) )
      .hidden(true)
      .description( "Stops the listening thread" );

  regist_signal("forward_signal")
      .hidden(true)
      .description("Called when there is a signal to forward");

  regist_signal( "message" )
      .description("New message has arrived from a worker")
      .pretty_name("")
      .connect( boost::bind(&Manager::signal_message, this, _1) );

  regist_signal( "signal_to_forward" )
      .description("Signal called by this object when to forward a signal "
                    "called from a worker.");

  signal("spawn_group")->signature( boost::bind(&Manager::signature_spawn_group, this, _1) );
  signal("kill_group")->signature( boost::bind(&Manager::signature_kill_group, this, _1) );

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
  signal("message")->hidden(true);
  signal("signal_to_forward")->hidden(true);

  m_listener->new_signal.connect( boost::bind(&Manager::new_signal, this, _1, _2) );
}

////////////////////////////////////////////////////////////////////////////

Manager::~Manager ()
{

}

////////////////////////////////////////////////////////////////////////////

void Manager::new_signal ( const ::MPI::Intercomm&, boost::shared_ptr<XML::XmlDoc> sig)
{
  if( Comm::instance().instance().get_parent() == MPI_COMM_NULL )
  {
    SignalFrame frame( sig );

    //std::cout << "forwarding frame " << frame.node.attribute_value("key") <<std::endl;
    call_signal( "signal_to_forward", frame );
  }
  else
  {
    SignalFrame signal_frame( sig );
    bool success = false;
    std::string message;

    std::string target = signal_frame.node.attribute_value("target");
    std::string receiver = signal_frame.node.attribute_value("receiver");

    try
    {

      if( target.empty() )
        throw ValueNotFound(FromHere(), "Could not find a valid target.");

      if( receiver.empty() )
        throw ValueNotFound(FromHere(), "Could not find a valid receiver.");

      std::string str;
      to_string( signal_frame.node, str);

      Handle<Component> comp = access_component_checked( receiver );

      comp->call_signal(target, signal_frame);

      if( Comm::instance().rank() == 0 ) // only rank 0 sends the reply
      {
        SignalFrame reply = signal_frame.get_reply();

        if( reply.node.is_valid() && !reply.node.attribute_value("target").empty() )
        {
          send_to_parent( signal_frame );

          //std::cout << "sending reply to frame " << signal_frame.node.attribute_value("key") <<std::endl;
        }
      }

      success = true;
    }
    catch( Exception & cfe )
    {
      message = cfe.what();
    }
    catch( std::exception & stde )
    {
      message = stde.what();
    }
    catch(...)
    {
      message = "Unhandled exception.";
    }

    if( Comm::instance().rank() == 0 )
    {
      /// @todo change the receiver path to be not hardcoded
      SignalFrame frame("ack", uri(), "cpath:/UI/NetworkQueue");
      SignalOptions & options = frame.options();
      std::string frameid = signal_frame.node.attribute_value("frameid");

      options.add("frameid", frameid );
      options.add("success", success );
      options.add("message", message );

      options.flush();

      m_queue->flush();

      send_to_parent( frame );
    }

    // synchronize with other buddies
    Comm::instance().barrier();
  }

}

////////////////////////////////////////////////////////////////////////////

void Manager::new_event ( SignalArgs & args )
{
  send_to_parent ( args );
}

////////////////////////////////////////////////////////////////////////////

void Manager::spawn_group ( const std::string & name,
                               Uint nb_workers,
                               const char * command,
                               const std::string & forward,
                               const char * hosts )
{
  if( m_groups.find(name) != m_groups.end())
    throw ValueExists(FromHere(), "A group of name " + name + " already exists.");

  boost::filesystem::path path;
  std::string forw = "--forward=" + forward;

  path = boost::filesystem::system_complete( command );

  // MPI wants the arguments to be 'char *' and not 'const char *'
  // thus, we need to make a copy since std::string::c_str() returns a const.
  char * forw_cstr = new char[forw.length() + 1];

  std::strcpy( forw_cstr, forw.c_str() );

  char * args[] = { forw_cstr, nullptr };

  Communicator comm = Comm::instance().spawn(nb_workers, command, args, hosts);
  m_groups[name] = comm;
  m_listener->add_communicator( comm );

  Handle<WorkerGroup> wg = create_component<WorkerGroup>(name);
  wg->set_communicator(comm);
  wg->mark_basic();

  Comm::instance().barrier( comm );

  // if it is the first group, we start listening
  if( m_groups.size() == 1 )
    m_listener->start_listening();


  delete forw_cstr;
}

////////////////////////////////////////////////////////////////////////////

void Manager::kill_group ( const std::string & name )
{
  SignalFrame frame("exit", uri(), uri());
  std::map<std::string, Communicator>::iterator it = m_groups.find(name);

  if( it == m_groups.end() )
    throw ValueNotFound(FromHere(), "Group [" + name + "] does not exist.");

  send_to( it->second, frame );

  // workers have a barrier on their parent comm just before calling MPI_Finalize
  Comm::instance().barrier( it->second );
  m_listener->remove_comunicator( it->second );

  m_groups.erase(it);

  // if there are no groups anymore, we stop the listening
  if( m_groups.empty() )
    m_listener->stop_listening();

  remove_component(it->first);

  CFinfo << "Group " << name << " was killed." << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void Manager::kill_all ()
{
  // stop the thread and wait() first
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////

void Manager::wait ()
{
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////

void Manager::send_to_parent ( const SignalArgs & args )
{
  send_to("MPI_Parent", args);
}

////////////////////////////////////////////////////////////////////////////

void Manager::send_to ( const std::string & group, const SignalArgs & args )
{
  std::map<std::string, Communicator>::iterator it = m_groups.find(group);

  if( it == m_groups.end() )
    throw ValueNotFound(FromHere(), "Group [" + group + "] does not exist.");

  send_to( it->second, args );
}

////////////////////////////////////////////////////////////////////////////

void Manager::broadcast ( const SignalArgs & args )
{
  std::map<std::string, Communicator>::iterator it = m_groups.begin();

  for( ; it != m_groups.end() ; ++it )
    send_to( it->second, args );
}

////////////////////////////////////////////////////////////////////////////

void Manager::send_to ( Communicator comm, const SignalArgs &args )
{
  std::string str;
  char * buffer;
  int remote_size;

  cf3_assert( is_not_null(args.xml_doc) );

  to_string( *args.xml_doc, str);

  buffer = new char[ str.length() + 1 ];
  std::strcpy( buffer, str.c_str() );

  MPI_Comm_remote_size(comm, &remote_size);

//  std::cout << "Worker[" << Comm::instance().rank() << "]" << " -> Sending " << buffer << std::endl;

  for(int i = 0 ; i < remote_size ; ++i)
    MPI_Send( buffer, str.length() + 1, MPI_CHAR, i, 0, comm );

  delete [] buffer;
}

////////////////////////////////////////////////////////////////////////////

boost::thread * Manager::listening_thread ()
{
  return &m_listener->thread();
}

////////////////////////////////////////////////////////////////////////////

void Manager::signal_spawn_group ( SignalArgs & args )
{
  SignalOptions options( args );
  const std::string cmd = std::string(CF3_BUILD_DIR) + "/src/Tools/solver/coolfluid-solver";

  Uint nb_workers = options.value<Uint>("count");
  std::string name = options.value<std::string>("name");
  std::string forward = options.value<std::string>("log_forwarding");

  if(forward == "None")
    forward = "none";
  else if (forward == "Only rank 0")
    forward = "rank0";
  else if (forward == "All ranks")
    forward = "all";
  else
    throw ValueNotFound(FromHere(), "Unknown forward type [" + forward + "]");

  spawn_group(name, nb_workers, cmd.c_str(), forward);
}

////////////////////////////////////////////////////////////////////////////

void Manager::signal_kill_group ( SignalArgs & args )
{
  SignalOptions options(args);
  std::string group_name = options.value<std::string>("group");

  kill_group( group_name );
}

////////////////////////////////////////////////////////////////////////////

void Manager::signal_kill_all ( SignalArgs & args )
{

}

////////////////////////////////////////////////////////////////////////////

void Manager::signal_message ( SignalArgs & args )
{
  SignalOptions options(args);

  std::string msg = options.value<std::string>("message");

//  CFinfo << msg << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void Manager::mpi_forward ( SignalArgs & args )
{
  boost::shared_ptr<XmlDoc> doc = Protocol::create_doc();
  XmlNode node = Protocol::goto_doc_node( *doc.get() );
  XmlNode sig_node = node.add_node( "tmp" );

  node.deep_copy( sig_node );
}

////////////////////////////////////////////////////////////////////////////

void Manager::signal_exit ( SignalArgs & args )
{
  m_listener->stop_listening();
}

////////////////////////////////////////////////////////////////////////////

void Manager::signature_spawn_group ( SignalArgs & args )
{
  SignalOptions options( args );

  options.add("name", std::string())
      .pretty_name("Name")
      .description("Name of the new group");

  options.add("count", Uint(1))
      .pretty_name("Workers Count")
      .description("Number of workers to spawn.");

  options.add("log_forwarding", std::string("None") )
      .pretty_name("Log Forwarding")
      .description("Defines the way the log is forwarded from the workers.")
      .restricted_list() += std::string("Only rank 0"), std::string("All ranks");

}

////////////////////////////////////////////////////////////////////////////

void Manager::signature_kill_group ( SignalArgs & args )
{
  SignalOptions options( args );
  std::vector<boost::any> groups( m_groups.size() - 1 );
  std::map<std::string, Communicator>::iterator it = m_groups.begin();

  if(m_groups.empty())
    throw IllegalCall(FromHere(), "There are no groups to kill.");

  for(int i = 0 ; it != m_groups.end() ; ++it, ++i )
    groups[i] = it->first;

  options.add("group", m_groups.begin()->first )
      .pretty_name("Group to kill")
      .description("Processes belonging to the selected group will be exited.")
      .restricted_list() = groups;
}

////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3
