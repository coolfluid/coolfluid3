// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Common/OptionT.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/NetworkThread.hpp"
#include "UI/Core/NetworkQueue.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NRoot.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////


NRoot::NRoot(const std::string & name)
  : CNode(name, "CRoot", CNode::STANDARD_NODE)
{
  m_isRoot = true;
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

  m_localSignals << "connect_server" << "disconnect_server";

  // signatures
  signal("connect_server")->signature( boost::bind(&NRoot::signature_connect_server, this, _1) );
  signal("disconnect_server")->signature( boost::bind(&NRoot::signature_disconnect_server, this, _1) );

  m_root = CRoot::create(name);

  connect(&ThreadManager::instance().network(), SIGNAL(connected()),
          this, SLOT(connectedToServer()));
  connect(&ThreadManager::instance().network(), SIGNAL(disconnectedFromServer()),
          this, SLOT(disconnected()));
}

////////////////////////////////////////////////////////////////////////////////

QString NRoot::toolTip() const
{
  return this->componentType();
}

////////////////////////////////////////////////////////////////////////////////

CNode::Ptr NRoot::childFromRoot(CF::Uint number) const
{
  ComponentIterator<CNode> it = m_root->begin<CNode>();
  CF::Uint i;

  for(i = 0 ; i < number && it != m_root->end<CNode>() ; i++)
    it++;

  // if number is bigger than the map size, it is equal to end()
  cf_assert(it != m_root->end<CNode>());

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

void NRoot::connectedToServer()
{
  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UUID %1.";

//  NLog::globalLog()->addMessage(msg1.arg(host).arg(port));
  NLog::globalLog()->addMessage(msg2.arg(uuid().c_str()));

  // build and send signal
  SignalFrame frame("client_registration", CLIENT_ROOT_PATH, SERVER_CORE_PATH);

  NetworkQueue::global_queue()->send( frame, NetworkQueue::IMMEDIATE );
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::disconnected()
{
  m_contentListed = false;
  m_actionSigs.clear();
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::shutdown(SignalArgs & node)
{
  NLog::globalLog()->addMessage("The server is shutting down. Disconnecting...");
  ThreadManager::instance().network().disconnectFromServer(false);
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::client_registration(SignalArgs & node)
{
  if( SignalOptions(node).value<bool>("accepted") )
  {
    NLog::globalLog()->addMessage("Registration was successful.");
    emit connected();
    NTree::globalTree()->updateTree();
  }
  else
  {
    NLog::globalLog()->addError("Registration failed. Disconnecting...");
    ThreadManager::instance().network().disconnectFromServer(false);
  }
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::frame_rejected(SignalArgs & args)
{
  SignalOptions options( args );

  std::string frameid = options.value<std::string>("frameid");
  std::string reason = options.value<std::string>("reason");

  QString msg("Action %1 has been rejected by the server: %2");

  NLog::globalLog()->addError(msg.arg(frameid.c_str()).arg(reason.c_str()));
}

////////////////////////////////////////////////////////////////////////////////

void NRoot::disableLocalSignals(QMap<QString, bool> & localSignals) const
{
  bool connected = ThreadManager::instance().network().isConnected();

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

  ThreadManager::instance().network().connectToHost(hostname.c_str(), port);
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_disconnect_server( SignalArgs & frame )
{
  bool shutdown = SignalOptions(frame).value<bool>("Shutdown the server");

  ThreadManager::instance().network().disconnectFromServer(shutdown);
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
