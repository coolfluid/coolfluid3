
#include <QtCore>

#include "Common/BasicExceptions.hpp"

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

//void ClientCore::newTree(const QDomDocument & domDocument)
//{
//  QModelIndex simulation = this->getSimIndex(sender());
//
//  if(m_treeModel != CFNULL && simulation.isValid())
//  {
//    m_treeModel->setSimulationTree(domDocument, simulation);
//    ClientRoot::getLog()->addMessage("Treeview updated.");
//  }
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::connected()
{
//  ClientNetworkComm * comm = static_cast<ClientNetworkComm*>(sender());
//  QModelIndex simulation = this->getSimIndex(this->sender());
//  TSshInformation sshInfo = m_commsSshInfo[simulation];
//  QString msg = "Now connected to server '%1' on port %2.";
//
//  if(m_timers[simulation]->isActive())
//  {
//    // stop the process (send SIGKILL signal)
//    m_launchServerProcs[simulation]->kill();
//    m_timers[simulation]->stop();
//    ClientRoot::getLog()->addMessage("Server started!");
//  }
//
//  m_treeModel->setSimConnectedStatus(simulation, true);
//  ClientRoot::getLog()->addMessage(msg.arg(sshInfo.m_hostname).arg(sshInfo.port));
//  comm->sendGetHostList();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void ClientCore::abstractTypes(const QStringList & types)
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::concreteTypes(const QStringList & types)
//{
//
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::tryToConnect()
{
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

//void ClientCore::hostList(const QList<HostInfos> & infos)
//{
//  QDomDocument doc;
//  QDomElement root = doc.createElement("hosts");
//  QList<HostInfos>::const_iterator it = infos.begin();
//  QModelIndex index = this->getSimIndex(sender());
//
//  while(it != infos.end())
//  {
//    QDomElement elem = doc.createElement("item");
//    QString text = "%1 (%2 slot(s))";
//
//    elem.setAttribute("name", it->m_hostname);
//    elem.setAttribute("selected", "false");
//
//    elem.appendChild(doc.createTextNode(text.arg(it->m_hostname).arg(it->m_nbSlots)));
//
//    root.appendChild(elem);
//
//    it++;
//  }
//
//  doc.appendChild(root);
//
//  m_treeModel->setSimulationHostList(index, doc);
//  ClientRoot::getLog()->addMessage(QString("Host list updated for simulation '%1'")
//                     .arg(m_treeModel->getNodePath(index)));
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::subsystemList(const QStringList & subSystems)
//{
//  QStringList::const_iterator it = subSystems.begin();
//  QModelIndex index = this->getSimIndex(sender());
//  int nbProcs;
//  QStringList hosts;
//
//  m_treeModel->getWorkersInfo(index, nbProcs, hosts);
//
//  while(it != subSystems.end())
//  {
//    m_statusModel->addSubSystem(*it, nbProcs);
//    it++;
//  }
//
//  m_networkComms[index]->sendActivateSimulation(nbProcs, hosts.join(","));
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::simulationStatus(const QString & subSysName, int rank,
//                                    const QString & status)
//{
//  m_statusModel->setWorkerStatus(subSysName, rank, status);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientCore::sshError()
{
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
//  if(m_networkComms.contains(index) && m_networkComms.value(index) != CFNULL
//     && m_networkComms[index]->isConnected())
  if(m_networkComm != CFNULL && m_networkComm->isConnected())
    ClientRoot::getLog()->addError("This simulation is already connected.");
  else
  {
    //ClientNetworkComm * comm = new ClientNetworkComm();
    //QTimer * timer = new QTimer(this);
    //QProcess * launchProcess = new QProcess(this);

    // forward some signals from the network layer to the upper level
//    connect(comm, SIGNAL(dirContents(const QString &, const QStringList &, const QStringList &)),
//            this, SIGNAL(dirContents(const QString &, const QStringList &, const QStringList &)));

    // connectSig is a macro defined at the top of this file
//    connectSig(comm, newTree(const QDomDocument &));
//    connectSig(comm, disconnectFromServer());
//    connectSig(comm, connected());
//    connectSig(comm, ack(CF::GUI::Network::NetworkFrameType));
//    connectSig(comm, nack(CF::GUI::Network::NetworkFrameType));
//    connectSig(comm, abstractTypes(const QStringList &));
//    connectSig(comm, concreteTypes(const QStringList &));
//    connectSig(comm, ack(CF::GUI::Network::NetworkFrameType));
//    connectSig(comm, nack(CF::GUI::Network::NetworkFrameType));
//    connectSig(comm, hostList(const QList<CF::GUI::Network::HostInfos> &));
//    connectSig(comm, simulationStatus(const QString &, int, const QString &));
//    connectSig(comm, subsystemList(const QStringList &));

//    if(m_networkComms.contains(index))
//      delete m_networkComms.value(index);
//
//    if(m_timers.contains(index))
//      delete m_timers.value(index);
//
//    if(m_launchServerProcs.contains(index))
//      delete m_launchServerProcs.value(index);

    //m_networkComms[index] = comm;

    //m_timers[index] = timer;
    //m_launchServerProcs[index] = launchProcess;
    //m_commsSshInfo[index] = info;

//    connect(timer, SIGNAL(timeout()), this, SLOT(tryToConnect()));
//    connect(launchProcess, SIGNAL(readyReadStandardError()), this, SLOT(sshError()));

    m_commSshInfo = info;
    this->connectToServer(index);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void ClientCore::disconnectSimulation(const QModelIndex & index, bool shutServer)
//{
//  if(!m_networkComms.contains(index) || m_networkComms.value(index) == CFNULL
//     || !m_networkComms[index]->isConnected())
//    ClientRoot::getLog()->addError("This simulation is not connected.");
//  else
//    m_networkComms[index]->disconnectFromServer(shutServer);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void ClientCore::runSimulation(const QModelIndex & index)
//{
//  m_networkComms[index]->sendRunSimulation();
//}
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::stopSimulation(const QModelIndex & index)
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::activateSimulation(const QModelIndex & index)
//{
//  if(!m_networkComms.contains(index) || m_networkComms.value(index) == CFNULL
//     || !m_networkComms[index]->isConnected())
//    ClientRoot::getLog()->addError("This simulation is not connected.");
//  else
//    m_networkComms[index]->sendGetSubSystemList();
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::deactivateSimulation(const QModelIndex & index)
//{
//  if(!m_networkComms.contains(index) || m_networkComms.value(index) == CFNULL
//     || !m_networkComms[index]->isConnected())
//    ClientRoot::getLog()->addError("This simulation is not connected.");
//  else
//    m_networkComms[index]->sendDeactivateSimulation();
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::disconnectFromServer()
//{
//  QModelIndex index = this->getSimIndex(sender());
//
//  if(index.isValid())
//  {
//    // ClientRoot::getLog()->addMessage("Disconnected from the server.");
//    m_treeModel->setSimConnectedStatus(index, false);
//  }
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::ack(NetworkFrameType type)
//{
//  QModelIndex index = this->getSimIndex(sender());
//
//  switch(type)
//  {
//    case NETWORK_OPEN_FILE:
//      m_networkComms[index]->sendGetTree();
//      break;
//
//    case NETWORK_ACTIVATE_SIMULATION:
//      m_treeModel->setSimActiveState(index, true);
//      break;
//
//    case NETWORK_DEACTIVATE_SIMULATION:
//      m_treeModel->setSimActiveState(index, false);
//      m_statusModel->clear();
//      break;
//
//    case NETWORK_SIMULATION_RUNNING:
//      m_treeModel->setSimReadOnly(index, true);
//      break;
//
//    case NETWORK_RUN_SIMULATION:
//      m_treeModel->setSimReadOnly(index, false);
//      break;
//
//    default:
//      emit acked(type);
//  }
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::nack(NetworkFrameType type)
//{
//  switch(type)
//  {
//    case NETWORK_ACTIVATE_SIMULATION:
//      m_statusModel->clear();
//      break;
//
//    default:
//      // ClientRoot::getLog()->addMessage("Unexpected NACK received");
//      break;
//  }
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::addComponent(const QModelIndex & index,
//                                ComponentType::Type type, const QString & name)
//{
//  cf_assert(type != ComponentType::INVALID && type != ComponentType::ROOT);
//
//  QString path = m_treeModel->getNodePathInSim(index);
//  ClientNetworkComm * comm = m_networkComms[m_treeModel->getParentSimIndex(index)];
//
//  path.prepend("/");
//
//  comm->sendAddComponent(path, type, name);
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::addLink(const QModelIndex & index, const QString & name,
//                           const QModelIndex & target)
//{
//  ClientNetworkComm * comm = m_networkComms[m_treeModel->getParentSimIndex(index)];
//
//  cf_assert(comm != CFNULL);
//
//  QString path = m_treeModel->getNodePathInSim(index);
//  QString targetPath = m_treeModel->getNodePathInSim(target);
//
//  comm->sendAddLink(path, name, targetPath);
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::addNode(const QString & abstractType)
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::renameNode(const QDomNode & node, const QString & newName)
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::deleteNode(const QDomNode & node)
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::commitChanges(const QDomDocument & doc)
//{
//  ClientNetworkComm * comm = m_networkComms[m_treeModel->getCurrentSimulation()];
//  comm->sendModifyNode(doc);
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void ClientCore::updateTree(const QModelIndex & index)
//{
//  ClientNetworkComm * comm = m_networkComms[m_treeModel->getCurrentSimulation()];
//  comm->sendGetTree();
//}
