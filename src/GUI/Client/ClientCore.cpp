
#include <QtCore>

#include "Common/BasicExceptions.hpp"

#include "GUI/Client/ClientNetworkComm.hpp"
#include "GUI/Client/NLog.hpp"
#include "GUI/Client/StatusModel.hpp"
#include "GUI/Client/TSshInformation.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/ClientCore.hpp"

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

ClientCore::ClientCore()
{
  m_timer = new QTimer(this);
  m_networkComm = new ClientNetworkComm();
  m_process = new QProcess(this);

  connect(m_networkComm, SIGNAL(connected()), this, SLOT(connected()));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ClientCore::~ClientCore()
{
  delete m_timer;
  delete m_networkComm;
  delete m_process;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ClientCore & ClientCore::instance()
{
  static ClientCore instance;
  return instance;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::sendSignal(const CF::Common::XmlDoc & signal)
{
  m_networkComm->send(signal);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::connectToServer(const TSshInformation & sshInfo)
{
  m_networkComm->connectToServer(sshInfo.m_hostname, sshInfo.port, false);
}

/****************************************************************************

 PRIVATE SLOTS

 ****************************************************************************/

void ClientCore::connected()
{
  QString msg = "Now connected to server '%1' on port %2.";
  ClientRoot::getLog()->addMessage(msg.arg(m_commSshInfo.m_hostname).arg(m_commSshInfo.port));

  boost::shared_ptr<XmlDoc> root = XmlOps::create_doc();

  XmlNode * docNode = XmlOps::goto_doc_node(*root.get());

  XmlOps::add_signal_frame(*docNode, "list_tree", CLIENT_TREE_PATH, SERVER_ROOT_PATH);

  m_networkComm->send(*root.get());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::tryToConnect()
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

void ClientCore::sshError()
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

