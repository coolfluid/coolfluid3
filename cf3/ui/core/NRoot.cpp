// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "common/Group.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/core/NetworkThread.hpp"
#include "ui/core/NetworkQueue.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/NRoot.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////////


NRoot::NRoot(const std::string & name)
  : CNode(name, "Root", CNode::STANDARD_NODE)
{
  m_is_root = true;
  m_uuid = boost::uuids::random_generator()();

  regist_signal( "shutdown" )
    ->description("Server shutdown")
    ->pretty_name("")->connect(boost::bind(&NRoot::shutdown, this, _1));

  regist_signal( "client_registration" )
    ->description("Registration confirmation")
    ->pretty_name("")->connect(boost::bind(&NRoot::client_registration, this, _1));

  regist_signal( "frame_rejected" )
    ->description("Frame rejected by the server")
    ->pretty_name("")->connect(boost::bind(&NRoot::frame_rejected, this, _1));

  regist_signal( "connect_server" )
    ->connect( boost::bind( &NRoot::signal_connect_server, this, _1 ) )
    ->description("Connects to the server")
    ->pretty_name("Connect to server");

  regist_signal( "disconnect_server" )
    ->connect( boost::bind( &NRoot::signal_disconnect_server, this, _1 ) )
    ->description("Disconnects from the server")
    ->pretty_name("Disconnect from server");

  m_local_signals << "connect_server" << "disconnect_server";

  // signatures
  signal("connect_server")->signature( boost::bind(&NRoot::signature_connect_server, this, _1) );
  signal("disconnect_server")->signature( boost::bind(&NRoot::signature_disconnect_server, this, _1) );

  connect(&ThreadManager::instance().network(), SIGNAL(connected()),
          this, SLOT(connected_to_server()));
  connect(&ThreadManager::instance().network(), SIGNAL(disconnected_from_server(bool)),
          this, SLOT(disconnected(bool)));
}

////////////////////////////////////////////////////////////////////////////////

QString NRoot::tool_tip() const
{
  return this->component_type();
}

////////////////////////////////////////////////////////////////////////////////

CNode::ConstPtr NRoot::child_from_root(cf3::Uint number) const
{
  ComponentIterator<CNode const> it = component_begin<CNode>(*this);
  ComponentIterator<CNode const> end = component_end<CNode>(*this);

  for(Uint i = 0 ; i < number && it != end ; i++)
    it++;

  // if number is bigger than the map size, it is equal to end()
  cf3_assert(it != end);

  return it.get();
}

////////////////////////////////////////////////////////////////////////////////

CNode::Ptr NRoot::child_from_root(cf3::Uint number)
{
  ComponentIterator<CNode> it = component_begin<CNode>(*this);
  ComponentIterator<CNode> end = component_end<CNode>(*this);

  for(Uint i = 0 ; i < number && it != end ; i++)
    it++;

  // if number is bigger than the map size, it is equal to end()
  cf3_always_assert(it != end);

  return it.get();
}

////////////////////////////////////////////////////////////////////////////////

std::string NRoot::uuid() const
{
  std::ostringstream ss;
  ss << m_uuid;
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::connected_to_server()
{
  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UuiD %1.";

//  NLog::global()->add_message(msg1.arg(host).arg(port));
  NLog::global()->add_message(msg2.arg(uuid().c_str()));

  // build and send signal
  SignalFrame frame("client_registration", CLIENT_ROOT_PATH, SERVER_core_PATH);

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::disconnected( bool requested)
{
  m_content_listed = false;
  m_action_sigs.clear();
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::shutdown(SignalArgs & node)
{
  NLog::global()->add_message("The server is shutting down. Disconnecting...");
  ThreadManager::instance().network().disconnect_from_server(false);
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::client_registration(SignalArgs & node)
{
  if( SignalOptions(node).value<bool>("accepted") )
  {
    NLog::global()->add_message("Registration was successful.");
    emit connected();
    NTree::global()->update_tree();
  }
  else
  {
    NLog::global()->add_error("Registration failed. Disconnecting...");
    ThreadManager::instance().network().disconnect_from_server(false);
  }
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::frame_rejected(SignalArgs & args)
{
  SignalOptions options( args );

  std::string frameid = options.value<std::string>("frameid");
  std::string reason = options.value<std::string>("reason");

  QString msg("Action %1 has been rejected by the server: %2");

  NLog::global()->add_error(msg.arg(frameid.c_str()).arg(reason.c_str()));
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::disable_local_signals(QMap<QString, bool> & localSignals) const
{
  bool connected = ThreadManager::instance().network().is_connected();

  localSignals["connect_server"] = !connected;
  localSignals["disconnect_server"] = connected;
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signature_connect_server( SignalArgs & frame )
{
  SignalOptions options( frame );

  options.add_option< OptionT<std::string> >("Hostname", std::string("localhost") )
      ->description("Name or IP address of the server.");

  options.add_option< OptionT<Uint> >("Port number", Uint(62784) )
      ->description("Port number the server is listening to.");
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signature_disconnect_server( SignalArgs & frame )
{
  SignalOptions options( frame );

  options.add_option< OptionT<bool> >("Shutdown the server", false )
      ->description("If checked, the server application will be closed.");
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_connect_server( SignalArgs & frame )
{
  SignalOptions options( frame );

  std::string hostname = options.value<std::string>("Hostname");
  Uint port = options.value<Uint>("Port number");

  ThreadManager::instance().network().connect_to_host(hostname.c_str(), port);
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_disconnect_server( SignalArgs & frame )
{
  bool shutdown = SignalOptions(frame).value<bool>("Shutdown the server");

  ThreadManager::instance().network().disconnect_from_server(shutdown);
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
