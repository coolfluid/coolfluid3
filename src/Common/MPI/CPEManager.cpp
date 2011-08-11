// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp> // for 'operator+=()'

#include "rapidxml/rapidxml.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/BoostFilesystem.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/ListeningThread.hpp"

#include "Common/MPI/CPEManager.hpp"
#include "Common/MPI/CWorkerGroup.hpp"

#include "Common/Log.hpp"

using namespace boost::assign; // bring 'operator+=()' into scope
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CPEManager, Component, LibCommon > CPEManager_Builder;

////////////////////////////////////////////////////////////////////////////

CPEManager::CPEManager( const std::string & name )
  : Component(name)
{
  m_listener = new ListeningThread();

  if( PE::instance().get_parent() != MPI::COMM_NULL )
  {
    m_groups["MPI_Parent"] = PE::instance().get_parent();
    m_listener->add_communicator( PE::instance().get_parent() );
    m_listener->start_listening();
  }

  regist_signal( "spawn_group" )
    ->description("Creates a new group of workers")
    ->pretty_name("Spawn new group")->connect( boost::bind(&CPEManager::signal_spawn_group, this, _1) );

  regist_signal( "kill_group" )
    ->description("Kills a group of workers")
    ->pretty_name("Kill group")->connect( boost::bind(&CPEManager::signal_kill_group, this, _1) );

  regist_signal( "kill_all" )
    ->description("Kills all groups of workers")
    ->hidden(true)
    ->pretty_name("Kill all groups")->connect( boost::bind(&CPEManager::signal_kill_all, this, _1) );

  regist_signal("exit")
      ->connect( boost::bind(&CPEManager::signal_exit, this, _1) )
      ->hidden(true)
      ->description( "Stops the listening thread" );

  regist_signal("forward_signal")
      ->hidden(true)
      ->description("Called when there is a signal to forward");

  regist_signal( "message" )
    ->description("New message has arrived from a worker")
    ->pretty_name("")->connect( boost::bind(&CPEManager::signal_message, this, _1) );

  signal("spawn_group")->signature( boost::bind(&CPEManager::signature_spawn_group, this, _1) );
  signal("kill_group")->signature( boost::bind(&CPEManager::signature_kill_group, this, _1) );

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
  signal("message")->hidden(true);

  m_listener->new_signal.connect( boost::bind(&CPEManager::new_signal, this, _1, _2) );
}

////////////////////////////////////////////////////////////////////////////

CPEManager::~CPEManager()
{

}

////////////////////////////////////////////////////////////////////////////

void CPEManager::new_signal ( const MPI::Intercomm &, XML::XmlDoc::Ptr sig)
{
  XmlNode nodedoc = Protocol::goto_doc_node(*sig.get());
  SignalFrame sig_frame( nodedoc.content->first_node() );
  rapidxml::xml_attribute<>* tmpAttr = sig_frame.node.content->first_attribute("target");


  if( is_null(tmpAttr) )
    throw ValueNotFound(FromHere(), "Could not find the target.");

  std::string target = tmpAttr->value();

  tmpAttr = sig_frame.node.content->first_attribute("receiver");

  if( is_null(tmpAttr) )
    throw ValueNotFound(FromHere(), "Could not find the receiver.");


  if( !m_root.expired() )
  {
    CRoot::Ptr root = m_root.lock();
    Component::Ptr comp = root->retrieve_component_checked( tmpAttr->value() );

    comp->call_signal(target, sig_frame);
  }

}

////////////////////////////////////////////////////////////////////////////

void CPEManager::spawn_group(const std::string & name, Uint nb_workers,
                             const char * command, const std::string & forward,
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

  Communicator comm = PE::instance().spawn(nb_workers, command, args, hosts);
  m_groups[name] = comm;
  m_listener->add_communicator( comm );

  CWorkerGroup & wg = create_component<CWorkerGroup>(name);
  wg.set_communicator(comm);
  wg.mark_basic();

  PE::instance().barrier( comm );

  // if it is the first group, we start listening
  if( m_groups.size() == 1 )
    m_listener->start_listening();


  delete forw_cstr;
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::kill_group( const std::string & name )
{
  SignalFrame frame("exit", uri(), uri());
  std::map<std::string, Communicator>::iterator it = m_groups.find(name);

  if( it == m_groups.end() )
    throw ValueNotFound(FromHere(), "Group [" + name + "] does not exist.");

  send_to( it->second, frame );

  // workers have a barrier on their parent comm just before calling MPI_Finalize
  PE::instance().barrier( it->second );
  m_listener->remove_comunicator( it->second );

  m_groups.erase(it);

  // if there are no groups anymore, we stop the listening
  if( m_groups.empty() )
    m_listener->stop_listening();

  remove_component(it->first);

  CFinfo << "Group " << name << " was killed." << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::kill_all()
{
  // stop the thread and wait() first
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::wait()
{
  /// @todo implement
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::send_to_parent( const SignalArgs & args )
{
  send_to("MPI_Parent", args);
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::send_to( const std::string & group, const SignalArgs & args )
{
  std::map<std::string, Communicator>::iterator it = m_groups.find(group);

  if( it == m_groups.end() )
    throw ValueNotFound(FromHere(), "Group [" + group + "] does not exist.");

  send_to( it->second, args );
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::broadcast( const SignalArgs & args )
{
  std::map<std::string, Communicator>::iterator it = m_groups.begin();

  for( ; it != m_groups.end() ; ++it )
    send_to( it->second, args );
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::send_to(Communicator comm, const SignalArgs &args)
{
  std::string str;
  char * buffer;
  int remote_size;

  to_string( *args.xml_doc, str);

  buffer = new char[ str.length() + 1 ];
  std::strcpy( buffer, str.c_str() );

  MPI_Comm_remote_size(comm, &remote_size);

//  std::cout << "Worker[" << PE::instance().rank() << "]" << " -> Sending " << buffer << std::endl;

  for(int i = 0 ; i < remote_size ; ++i)
    MPI_Send( buffer, str.length() + 1, MPI_CHAR, i, 0, comm );

  delete [] buffer;
}

////////////////////////////////////////////////////////////////////////////

boost::thread & CPEManager::listening_thread()
{
  return m_listener->thread();
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signal_spawn_group ( SignalArgs & args )
{
  SignalOptions options( args );
  const char * cmd = "../Tools/Solver/coolfluid-solver";

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

  spawn_group(name, nb_workers, cmd, forward);
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signal_kill_group ( SignalArgs & args )
{
  SignalOptions options(args);
  std::string group_name = options.value<std::string>("group");

  kill_group( group_name );
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signal_kill_all ( SignalArgs & args )
{

}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signal_message ( SignalArgs & args )
{
  SignalOptions options(args);

  std::string msg = options.value<std::string>("message");

//  CFinfo << msg << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::mpi_forward ( SignalArgs & args )
{
  XmlDoc::Ptr doc = Protocol::create_doc();
  XmlNode node = Protocol::goto_doc_node( *doc.get() );
  XmlNode sig_node = node.add_node( "tmp" );

  node.deep_copy( sig_node );
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signal_exit( SignalArgs & args )
{
  m_listener->stop_listening();
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signature_spawn_group ( SignalArgs & args )
{
  SignalOptions options( args );

  options.add_option< OptionT<std::string> >("name", std::string())
      ->pretty_name("Name")
      ->description("Name of the new group");

  options.add_option< OptionT<Uint> >("count", Uint(1))
      ->pretty_name("Workers Count")
      ->description("Number of workers to spawn.");

  options.add_option< OptionT<std::string> >("log_forwarding", std::string("None") )
      ->pretty_name("Log Forwarding")
      ->description("Defines the way the log is forwarded from the workers.")
      ->restricted_list() += std::string("Only rank 0"), std::string("All ranks");

}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signature_kill_group ( SignalArgs & args )
{
  SignalOptions options( args );
  std::vector<boost::any> groups( m_groups.size() - 1 );
  std::map<std::string, Communicator>::iterator it = m_groups.begin();

  if(m_groups.empty())
    throw IllegalCall(FromHere(), "There are no groups to kill.");

  for(int i = 0 ; it != m_groups.end() ; ++it, ++i )
    groups[i] = it->first;

  options.add_option< OptionT<std::string> >("group", m_groups.begin()->first )
      ->pretty_name("Group to kill")
      ->description("Processes belonging to the selected group will be exited.")
      ->restricted_list() = groups;
}

////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF
