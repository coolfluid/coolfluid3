// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/BoostFilesystem.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/ListeningThread.hpp"

#include "Common/MPI/CPEManager.hpp"

#include "Common/Log.hpp"

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

  regist_signal("spawn_group", "Creates a new group of workers.", "Spawn new group")
      ->signal->connect( boost::bind(&CPEManager::signal_spawn_group, this, _1) );
  regist_signal("kill_group", "Kills a group of workers.", "Kill group")
      ->signal->connect( boost::bind(&CPEManager::signal_kill_group, this, _1) );
  regist_signal("kill_all", "Kills all groups of workers.", "Kill all groups")
      ->signal->connect( boost::bind(&CPEManager::signal_kill_all, this, _1) );
  regist_signal("exit", "Stops the listening thread", "")
      ->signal->connect( boost::bind(&CPEManager::signal_exit, this, _1) );

  regist_signal("forward_signal", "Called when there is a signal to forward,")->is_hidden = true;

  regist_signal("message", "New message has arrived from a worker")
      ->signal->connect( boost::bind(&CPEManager::signal_message, this, _1) );

  signal("spawn_group")->signature->connect( boost::bind(&CPEManager::signature_spawn_group, this, _1) );
  signal("kill_group")->signature->connect( boost::bind(&CPEManager::signature_kill_group, this, _1) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;
  signal("message")->is_hidden = true;

  // temporary hide
  signal("exit")->is_hidden = true;
  signal("kill_all")->is_hidden = true;

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

  m_groups[name] = PE::instance().spawn(nb_workers, command, args, hosts);
  m_listener->add_communicator( m_groups[name] );

  PE::instance().barrier();
  MPI_Barrier( m_groups[name] );

  // if it is the first group, we start listening
  if( m_groups.size() == 1 )
    m_listener->start_listening();

  delete forw_cstr;
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::kill_group( const std::string & name )
{
  SignalFrame frame("exit", full_path(), full_path());
  std::map<std::string, Communicator>::iterator it = m_groups.find(name);

  if( it == m_groups.end() )
    throw ValueNotFound(FromHere(), "Group [" + name + "] does not exist.");

  send_to( it->second, frame );

  // workers have a barrier on their parent comm just before calling MPI_Finalize
  MPI_Barrier( it->second );

  m_listener->remove_comunicator( it->second );

  m_groups.erase(it);

  // if there are no groups anymore, we stop the listening
  if( m_groups.empty() )
    m_listener->stop_listening();

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
  const char * cmd = "../../Tools/Solver/coolfluid-solver";

  Uint nb_workers = options.option<Uint>("Workers Count");
  std::string name = options.option<std::string>("Name");
  std::string forward = options.option<std::string>("Log Forwarding");

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
  std::string group_name = options.option<std::string>("Group to kill");

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

  std::string msg = options.option<std::string>("message");

  CFinfo << msg << CFendl;
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
  std::vector<std::string> forwards(3);

  forwards[0] = "None";
  forwards[1] = "Only rank 0";
  forwards[2] = "All ranks";

  options.add("Name", std::string(), "Name of the new group.");
  options.add("Workers Count", Uint(0), "Number of workers to spawn.");
  options.add("Log Forwarding", std::string("None"), "Defines the way the log is forwarded from the workers.",
              forwards, ";");
}

////////////////////////////////////////////////////////////////////////////

void CPEManager::signature_kill_group ( SignalArgs & args )
{
  SignalOptions options( args );
  std::vector<std::string> groups( m_groups.size() );
  std::map<std::string, Communicator>::iterator it = m_groups.begin();

  if(m_groups.empty())
    throw IllegalCall(FromHere(), "There are no groups to kill.");

  for(int i = 0 ; it != m_groups.end() ; ++it, ++i )
    groups[i] = it->first;

  options.add<std::string>("Group to kill", groups[0],
                           "Processes belonging to the selected group will be exited.", groups, ";");
}

////////////////////////////////////////////////////////////////////////////

} // mpi
} // Common
} // CF
