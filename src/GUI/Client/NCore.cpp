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


/****************************************************************************

 PRIVATE SLOTS

 ****************************************************************************/

void NCore::connected()
{
  QString msg = "Now connected to server '%1' on port %2.";
  ClientRoot::getLog()->addMessage(msg.arg(m_commSshInfo.m_hostname).arg(m_commSshInfo.port));

  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();

  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

  XmlOps::add_signal_frame(*docNode, "list_tree", CLIENT_TREE_PATH, SERVER_ROOT_PATH);

  m_networkComm->send(*root.get());

  emit connectedToServer();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::tryToConnect()
{
  throw NotImplemented(FromHere(), "ClientCore::tryToConnect");
//  QModelIndex index = m_timers.key(static_cast<QTimer*>(sender()));
//
//  if(index.isValid())
//  {
//    TSshInformation & sshInfo = m_commsSshInfo[index];
//    m_networkComms[index]->connectToServer(sshInfo.m_hostname, sshInfo.port, true);
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::sshError()
{
  throw NotImplemented(FromHere(), "ClientCore::sshError");
//  QProcess * process = static_cast<QProcess *>(sender());
//  QModelIndex index = m_launchServerProcs.key(process);
//
//  if(process != CFNULL && index.isValid())
//  {
//    m_timers[index]->stop();
//
//    QString errorMsg = m_launchServerProcs[index]->readAllStandardError();
//    ClientRoot::getLog()->addError(errorMsg);
//
//    // stop the process (send SIGKILL-like signal)
//    m_launchServerProcs[index]->kill();
//  }
//  else
//  {
//    QString errorMsg = "Unexpected call of ClientCore::sshError() from a "
//    "sender of type: %1";
//    if(sender() != CFNULL)
//      ClientRoot::getLog()->addError(errorMsg.arg(sender()->metaObject()->className()));
//    else
//      ClientRoot::getLog()->addError(errorMsg.arg("<unknown type>"));
//  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NCore::shutdown(CF::Common::XmlNode & node)
{
  ClientRoot::getLog()->addMessage("The server is shutting down. Disconnecting...");
  this->disconnectFromServer(false);
}
