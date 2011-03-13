// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QMutexLocker>
#include <QTimer>
#include <QProcess>

#include "Common/BasicExceptions.hpp"
#include "Common/Signal.hpp"

#include "GUI/Client/Core/ClientNetworkComm.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NCore.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::Network;

NCore::NCore()
  : CNode(CLIENT_CORE, "NCore", CNode::CORE_NODE)
{
  m_networkComm = new ClientNetworkComm();

  connect(m_networkComm, SIGNAL(connected()), this, SLOT(connected()));
  connect(m_networkComm, SIGNAL(disconnectedFromServer()), this, SLOT(disconnected()));

  regist_signal("shutdown", "Server shutdown")->signal->connect(boost::bind(&NCore::shutdown, this, _1));
  regist_signal("client_registration", "Registration confirmation")->signal->connect(boost::bind(&NCore::client_registration, this, _1));
  regist_signal("frame_rejected", "Frame rejected by the server")->signal->connect(boost::bind(&NCore::frame_rejected, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NCore::~NCore()
{
  delete m_networkComm;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::sendSignal(SignalArgs & signal)
{
  m_networkComm->send(signal);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::connectToServer(const TSshInformation & sshInfo)
{
  m_networkComm->connectToServer(sshInfo.m_hostname, sshInfo.m_port, false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::disconnectFromServer(bool shutdown)
{
  m_networkComm->disconnectFromServer(shutdown);

  disconnected();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::disconnected()
{
  NTree::globalTree()->setCurrentIndex(QModelIndex());

  NLog::globalLog()->addMessage("Disconnected from the server.");

  emit disconnectedFromServer();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NCore::toolTip() const
{
  return this->getComponentType();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::updateTree()
{
  SignalFrame frame("list_tree", CLIENT_TREE_PATH, SERVER_ROOT_PATH);
  m_networkComm->send(frame);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NCore::isConnected()
{
  return m_networkComm->isConnected();
}

/****************************************************************************

 PRIVATE SLOTS

 ****************************************************************************/

void NCore::connected()
{
  // get some reference (for better readability)
  QString & host = m_commSshInfo.m_hostname;
  quint16 & port = m_commSshInfo.m_port;
  std::string uuid = ClientRoot::instance().getUUID();

  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UUID %1.";

  NLog::globalLog()->addMessage(msg1.arg(host).arg(port));
  NLog::globalLog()->addMessage(msg2.arg(uuid.c_str()));

  // build and send signal
  SignalFrame frame("client_registration", CLIENT_CORE_PATH, SERVER_CORE_PATH);

  m_networkComm->send(frame);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::shutdown(SignalArgs & node)
{
  NLog::globalLog()->addMessage("The server is shutting down. Disconnecting...");
  this->disconnectFromServer(false);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::client_registration(SignalArgs & node)
{
  if( node.map(Protocol::Tags::key_options()).get_option<bool>("accepted") )
  {
    NLog::globalLog()->addMessage("Registration was successful.");
    m_networkComm->saveNetworkInfo();
    emit connectedToServer();
    this->updateTree();
  }
  else
  {
    NLog::globalLog()->addError("Registration failed. Disconnecting...");
    this->disconnectFromServer(false);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::frame_rejected(SignalArgs & args)
{
  SignalFrame & options = args.map( Protocol::Tags::key_options() );

  string frameid = options.get_option<string>("frameid");
  string reason = options.get_option<string>("reason");

  QString msg("Action %1 has been rejected by the server: %2");

  NLog::globalLog()->addError(msg.arg(frameid.c_str()).arg(reason.c_str()));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NCore::Ptr NCore::globalCore()
{
  static NCore::Ptr core = ClientRoot::instance().rootChild<NCore>(CLIENT_CORE);
  cf_assert( is_not_null(core.get()) );

  return core;
}
