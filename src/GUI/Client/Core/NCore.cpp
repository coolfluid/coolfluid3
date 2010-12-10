// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QFileIconProvider>
#include <QTimer>
#include <QProcess>

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/Core/ClientNetworkComm.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NCore.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::Network;

NCore::NCore()
  : CNode(CLIENT_CORE, "NCore", CNode::CORE_NODE)
{
  m_timer = new QTimer(this);
  m_networkComm = new ClientNetworkComm();
  m_process = new QProcess(this);

  connect(m_networkComm, SIGNAL(connected()), this, SLOT(connected()));
  connect(m_networkComm, SIGNAL(disconnectedFromServer()), this, SLOT(disconnected()));

  regist_signal("shutdown", "Server shutdown")->connect(boost::bind(&NCore::shutdown, this, _1));
  regist_signal("client_registration", "Registration confirmation")->connect(boost::bind(&NCore::client_registration, this, _1));
  regist_signal("frame_rejected", "Frame rejected by the server")->connect(boost::bind(&NCore::frame_rejected, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NCore::~NCore()
{
  delete m_timer;
  delete m_networkComm;
  delete m_process;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::sendSignal(XmlDoc & signal)
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
  ClientRoot::tree()->setCurrentIndex(QModelIndex());

  ClientRoot::log()->addMessage("Disconnected from the server.");

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
  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();
  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

  XmlOps::add_signal_frame(*docNode, "list_tree", CLIENT_TREE_PATH,
                           SERVER_ROOT_PATH, true);
  m_networkComm->send(*root.get());
}

/****************************************************************************

 PRIVATE SLOTS

 ****************************************************************************/

void NCore::connected()
{
  // get some reference (for better readability)
  QString & host = m_commSshInfo.m_hostname;
  quint16 & port = m_commSshInfo.m_port;
  std::string uuid = ClientRoot::getUUID();

  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UUID %1.";

  ClientRoot::log()->addMessage(msg1.arg(host).arg(port));
  ClientRoot::log()->addMessage(msg2.arg(uuid.c_str()));

  // build and send signal
  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();
  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

  XmlOps::add_signal_frame(*docNode, "client_registration", CLIENT_CORE_PATH,
                           SERVER_ROOT_PATH, true);

  m_networkComm->send(*root.get());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::shutdown(XmlNode & node)
{
  ClientRoot::log()->addMessage("The server is shutting down. Disconnecting...");
  this->disconnectFromServer(false);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::client_registration(XmlNode & node)
{
  XmlParams p(node);

  if(p.get_option<bool>("accepted"))
  {
    ClientRoot::log()->addMessage("Registration was successful.");
    m_networkComm->saveNetworkInfo();
    emit connectedToServer();
    this->updateTree();
  }
  else
  {
    ClientRoot::log()->addError("Registration failed. Disconnecting...");
    this->disconnectFromServer(false);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::frame_rejected(CF::Common::XmlNode & node)
{
  XmlParams p(node);
  string frameid = p.get_option<string>("frameid");
  string reason = p.get_option<string>("reason");

  QString msg("Action %1 has been rejected by the server: %2");

  ClientRoot::log()->addError(msg.arg(frameid.c_str()).arg(reason.c_str()));
}
