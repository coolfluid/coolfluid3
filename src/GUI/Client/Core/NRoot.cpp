// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NetworkThread.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NTree.hpp"
#include "GUI/Client/Core/ThreadManager.hpp"

#include "GUI/Client/Core/NRoot.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////


NRoot::NRoot(const QString & name)
  : CNode(name, "CRoot", ROOT_NODE)
{
  m_uuid = boost::uuids::random_generator()();

  regist_signal("shutdown", "Server shutdown")->signal->
      connect(boost::bind(&NRoot::shutdown, this, _1));
  regist_signal("client_registration", "Registration confirmation")->signal->
      connect(boost::bind(&NRoot::client_registration, this, _1));
  regist_signal("frame_rejected", "Frame rejected by the server")->signal->
      connect(boost::bind(&NRoot::frame_rejected, this, _1));

  regist_signal("connect_server", "Connects to the server", "Connect to server")->signal
      ->connect( boost::bind(&NRoot::signal_connect_server, this, _1));
  regist_signal("disconnect_server", "Disconnects from the server", "Disconnect from server")->signal
      ->connect( boost::bind(&NRoot::signal_disconnect_server, this, _1));

  m_localSignals << "connect_server" << "disconnect_server";

  // signatures
  signal("connect_server")->signature->connect( boost::bind(&NRoot::signature_connect_server, this, _1) );
  signal("disconnect_server")->signature->connect( boost::bind(&NRoot::signature_disconnect_server, this, _1) );

  m_root = CRoot::create(name.toStdString());

  connect(&ThreadManager::instance().network(), SIGNAL(connected()),
          this, SLOT(connectedToServer()));
  connect(&ThreadManager::instance().network(), SIGNAL(disconnectedFromServer()),
          this, SLOT(disconnected()));
}

////////////////////////////////////////////////////////////////////////////////

QString NRoot::toolTip() const
{
  return this->getComponentType();
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
  // get some reference (for better readability)

  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UUID %1.";

//  NLog::globalLog()->addMessage(msg1.arg(host).arg(port));
  NLog::globalLog()->addMessage(msg2.arg(uuid().c_str()));

  // build and send signal
  SignalFrame frame("client_registration", CLIENT_ROOT_PATH, SERVER_CORE_PATH);

  ThreadManager::instance().network().send(frame);
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
  if( node.map(Protocol::Tags::key_options()).get_option<bool>("accepted") )
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
  SignalFrame & options = args.map( Protocol::Tags::key_options() );

  std::string frameid = options.get_option<std::string>("frameid");
  std::string reason = options.get_option<std::string>("reason");

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
  SignalFrame p = frame.map( Protocol::Tags::key_options() );

  p.set_option("Hostname", std::string("localhost"), "Name or IP address of the server.");
  p.set_option("Port number", Uint(62784), "Port number the server is listening to.");
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signature_disconnect_server( SignalArgs & frame )
{
  SignalFrame p = frame.map( Protocol::Tags::key_options() );

  p.set_option("Shutdown the server", false, "If checked, the server application will be closed.");
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_connect_server( SignalArgs & frame )
{
  SignalFrame p = frame.map( Protocol::Tags::key_options() );

  std::string hostname = p.get_option<std::string>("Hostname");
  Uint port = p.get_option<Uint>("Port number");

  ThreadManager::instance().network().connectToHost(hostname.c_str(), port);
}

////////////////////////////////////////////////////////////////////////////

void NRoot::signal_disconnect_server( SignalArgs & frame )
{
  bool shutdown = frame.map( Protocol::Tags::key_options() ).get_option<bool>("Shutdown the server");

  ThreadManager::instance().network().disconnectFromServer(shutdown);
}

////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF
