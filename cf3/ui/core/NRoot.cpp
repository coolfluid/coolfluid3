// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "common/Group.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/UUCount.hpp"

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

  regist_signal( "shutdown" )
    .description("Server shutdown")
    .pretty_name("").connect(boost::bind(&NRoot::shutdown, this, _1));

  regist_signal( "client_registration" )
    .description("Registration confirmation")
    .pretty_name("").connect(boost::bind(&NRoot::client_registration, this, _1));

  regist_signal( "frame_rejected" )
    .description("Frame rejected by the server")
    .pretty_name("").connect(boost::bind(&NRoot::frame_rejected, this, _1));

  regist_signal( "connect_server" )
    .connect( boost::bind( &NRoot::signal_connect_server, this, _1 ) )
    .description("Connects to the server")
    .pretty_name("Connect to server");

  regist_signal( "disconnect_server" )
    .connect( boost::bind( &NRoot::signal_disconnect_server, this, _1 ) )
    .description("Disconnects from the server")
    .pretty_name("Disconnect from server");

  m_local_signals << "connect_server" << "disconnect_server";

  // signatures
  signal("connect_server")
      ->signature( boost::bind(&NRoot::signature_connect_server, this, _1) );
  signal("disconnect_server")
      ->signature( boost::bind(&NRoot::signature_disconnect_server, this, _1) );

  NetworkThread & ntwork = ThreadManager::instance().network();

  ntwork.signal( "network_connected" )
      ->connect( boost::bind(&NRoot::network_connected, this, _1) );
  ntwork.signal( "network_disconnected" )
      ->connect( boost::bind(&NRoot::network_disconnected, this, _1) );

//  connect(&ThreadManager::instance().network(), SIGNAL(connected()),
//          this, SLOT(network_connected()));
//  connect(&ThreadManager::instance().network(), SIGNAL(network_disconnected(bool)),
//          this, SLOT(disconnected(bool)));
}

////////////////////////////////////////////////////////////////////////////////

QString NRoot::tool_tip() const
{
  return this->component_type();
}

////////////////////////////////////////////////////////////////////////////////

Handle< CNode const > NRoot::child_from_root(cf3::Uint number) const
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

Handle< CNode > NRoot::child_from_root(cf3::Uint number)
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

const UUCount& NRoot::uuid() const
{
  return m_uuid;
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::network_connected( SignalArgs & )
{
  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UuiD %1.";

//  NLog::global()->add_message(msg1.arg(host).arg(port));
  NLog::global()->add_message(msg2.arg(uuid().string().c_str()));

  // build and send signal
  SignalFrame frame("client_registration", CLIENT_ROOT_PATH, SERVER_CORE_PATH);

  NetworkQueue::global()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::network_disconnected( SignalArgs & )
{
  m_content_listed = false;
  m_action_sigs.clear();
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::shutdown(SignalArgs & )
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

  options.add("hostname", std::string("localhost") )
      .pretty_name( "Hostname" )
      .description( "Name or IP address of the server." );

  options.add("port_number", Uint(62784) )
      .pretty_name( "Port Number" )
      .description( "Port number the server is listening to." );
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signature_disconnect_server( SignalArgs & frame )
{
  SignalOptions options( frame );

  options.add("shutdown", false )
      .pretty_name( "Shutdown the server" )
      .description( "If checked, the server application will be closed." );
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_connect_server( SignalArgs & frame )
{
  SignalOptions options( frame );

  std::string hostname = options.value<std::string>( "hostname" );
  Uint port = options.value<Uint>( "port_number" );

  ThreadManager::instance().network().connect_to_host( hostname.c_str(), port );
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_disconnect_server( SignalArgs & frame )
{
  bool shutdown = SignalOptions(frame).value<bool>( "shutdown" );

  ThreadManager::instance().network().disconnect_from_server(shutdown);
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
