
#include <QtCore>

#include "rapidxml/rapidxml_print.hpp"

#include "Common/BasicExceptions.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/NetworkFrameType.hpp"

#include "GUI/Client/ClientNetworkComm.hpp"
#include "GUI/Client/CLog.hpp"
#include "GUI/Client/StatusModel.hpp"
#include "GUI/Client/TSshInformation.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Client/ClientCore.hpp"

#define connectSig(comm,slotSig) connect(comm, SIGNAL(slotSig), this, SLOT(slotSig));

using namespace CF::Common;
using namespace CF::GUI::Client;
using namespace CF::GUI::Network;

ClientCore::ClientCore()
  : m_treeModel(CFNULL),
    m_statusModel(CFNULL)
{
  m_timer = new QTimer(this);
  m_networkComm = new ClientNetworkComm();
  m_process = new QProcess(this);

  connect (m_networkComm, SIGNAL(connected()), this, SLOT(connected()));
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

ClientCore & ClientCore::getInstance()
{
  static ClientCore instance;
  return instance;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::setTreeModel(TreeModel * treeModel)
{
  if(m_treeModel != CFNULL)
    m_treeModel->disconnect(this);
    disconnect(m_treeModel);

  m_treeModel = treeModel;

  if(m_treeModel != CFNULL)
  {
    /// @todo connect signals (if any)
  }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::setStatusModel(StatusModel * statusModel)
{
  if(m_statusModel != CFNULL)
    disconnect(m_statusModel);

  m_statusModel = statusModel;

  if(m_statusModel != CFNULL)
  {
    /// @todo connect signals (if any)
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TreeModel * ClientCore::getTreeModel() const
{
  return m_treeModel;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

StatusModel * ClientCore::getStatusModel() const
{
  return m_statusModel;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::sendSignal(const SignalInfo & signal)
{
  m_networkComm->send(signal);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::buildAndSendSignal(const QString & type, const CPath & sender,
                                    const CPath & receiver)
{
  SignalInfo si(type, sender, receiver, true);

  this->sendSignal(si);
}

/****************************************************************************

 PRIVATE METHODS

 ****************************************************************************/

void ClientCore::connectToServer(const QModelIndex & simIndex)
{
  m_networkComm->connectToServer(m_commSshInfo.m_hostname, m_commSshInfo.port, m_timer->isActive());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::launchServer(const QModelIndex & simIndex)
{
  throw NotImplemented(FromHere(), "ClientCore::launchServer");
//  QString cmd;
//  QString msg = "Checking if no other server instance is running on "
//  "\"%1\" on port \"%2\"...";
//  QProcess checkIfRunning;
//  TSshInformation & sshInfo = m_commsSshInfo[simIndex];
//
//  //
//  // checking if the remote port is available
//  //
//
//  cmd = QString("ssh %1@%2 check_coolfluid_server.sh %3")
//  .arg(sshInfo.username)
//  .arg(sshInfo.m_hostname)
//  .arg(sshInfo.port);
//
//  ClientRoot::getLog()->addMessage(msg.arg(sshInfo.m_hostname).arg(sshInfo.port));
//  checkIfRunning.start(cmd);
//
//  // wait that the process is done
//  checkIfRunning.waitForFinished(-1);
//
//  QString output = checkIfRunning.readAllStandardOutput();
//  QString errorMsg = checkIfRunning.readAllStandardError();
//
//  // if there is something on the stderr, the check failed
//  if(!errorMsg.isEmpty())
//  {
//    ClientRoot::getLog()->addError(errorMsg);
//    return ;
//  }
//
//  // if output is different from "0", a server is already running on this port
//  if(output != "0")
//  {
//    QString msg = "A server is already running on port %1 on %2. Please change "
//    "the port or the hostname.";
//
//    ClientRoot::getLog()->addError(msg.arg(sshInfo.m_hostname).arg(sshInfo.port));
//    return ;
//  }
//
//  //
//  // launching the server application
//  //
//
//  cmd = QString("ssh -n %1@%2 start_coolfluid_server.sh %3")
//  .arg(sshInfo.username)
//  .arg(sshInfo.m_hostname)
//  .arg(sshInfo.port);
//
//  ClientRoot::getLog()->addMessage("Starting the server...");
//
//  // client will try every 100 ms to connect to the server
//  m_timers[simIndex]->start(100);
//
//  m_launchServerProcs[simIndex]->start(cmd);
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

  XmlNode * frameNode = XmlOps::add_node_to ( *docNode, "frame" );
  XmlAttr * attrType = XmlOps::add_attribute_to(*frameNode, "type", "signal");
  XmlAttr * attrSender = XmlOps::add_attribute_to(*frameNode, "sender", CLIENT_LOG_PATH);
  XmlAttr * attrReceiver = XmlOps::add_attribute_to(*frameNode, "receiver", SERVER_ROOT_PATH);
  XmlAttr * attrTarget = XmlOps::add_attribute_to(*frameNode, "target", "list_tree");

//  XmlParams p(*frameNode);

//  p.add_param<std::string>("test", "hello world!");

//  qDebug() << "BEFORE";
//  std::string s;
//  XmlOps::xml_to_string(*root.get(), s);
//  qDebug() << s.c_str();
//  qDebug() << "AFTER";

  m_networkComm->send(*root.get());

//  SignalInfo si("list_signal", CLIENT_LOG_PATH, SERVER_CORE_PATH, true);
//  m_networkComm->send(si);
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

/****************************************************************************

 PUBLIC SLOTS

 ****************************************************************************/

void ClientCore::connectSimulation(const QModelIndex & index,
                                     const TSshInformation & info)
{
  if(m_networkComm != CFNULL && m_networkComm->isConnected())
    ClientRoot::getLog()->addError("This simulation is already connected.");
  else
  {
    m_commSshInfo = info;
    this->connectToServer(index);
  }
}
