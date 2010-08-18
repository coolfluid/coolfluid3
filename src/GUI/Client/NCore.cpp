#include <QtCore>
#include <QtGui>

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/ClientNetworkComm.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/StatusModel.hpp"
#include "GUI/Client/TSshInformation.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NCore.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

NCore::NCore()
  : CNode(CLIENT_CORE, "NCore", CNode::CORE_NODE)
{
  BUILD_COMPONENT;

  m_timer = new QTimer(this);
  m_networkComm = new ClientNetworkComm();
  m_process = new QProcess(this);

  connect(m_networkComm, SIGNAL(connected()), this, SLOT(connected()));

  regist_signal("shutdown", "Server shutdown")->connect(boost::bind(&NCore::shutdown, this, _1));
  regist_signal("client_registration", "Server shutdown")->connect(boost::bind(&NCore::client_registration, this, _1));
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
  m_networkComm->connectToServer(sshInfo.m_hostname, sshInfo.port, false);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::disconnectFromServer(bool shutdown)
{
  m_networkComm->disconnectFromServer(shutdown);

  ClientRoot::getLog()->addMessage("Disconnected from the server.");

  emit disconnectedFromServer();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NCore::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NCore::getToolTip() const
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
  quint16 & port = m_commSshInfo.port;
  std::string uuid = ClientRoot::getUUID();

  QString msg1 = "Now connected to server '%1' on port %2.";
  QString msg2 = "Attempting to register with UUID %1.";

  ClientRoot::getLog()->addMessage(msg1.arg(host).arg(port));
  ClientRoot::getLog()->addMessage(msg2.arg(uuid.c_str()));

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
  ClientRoot::getLog()->addMessage("The server is shutting down. Disconnecting...");
  this->disconnectFromServer(false);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::client_registration(XmlNode & node)
{
  XmlParams p(node);

  if(p.get_param<bool>("accepted"))
  {
    ClientRoot::getLog()->addMessage("Registration was successful.");
    emit connectedToServer();
    this->updateTree();
  }
  else
  {
    ClientRoot::getLog()->addError("Registration failed. Disconnecting...");
    this->disconnectFromServer(false);
  }
}
