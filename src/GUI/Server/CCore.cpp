#include <QtXml>
#include <QHostInfo>
#include <mpi.h>

#include <boost/filesystem/path.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/xmlParser.h"
#include "Common/MPI/PEInterface.hpp"
#include "Common/Log.hpp"
#include "Common/ConfigArgs.hpp"
#include "Common/ConverterTools.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/HostInfos.hpp"
#include "GUI/Network/SignalInfo.hpp"

#include "GUI/Server/ServerNetworkComm.hpp"
#include "GUI/Server/CSimulator.hpp"
#include "GUI/Server/SimulationManager.hpp"
#include "GUI/Server/TypesNotFoundException.hpp"

#include "GUI/Server/CCore.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;
using namespace CF::GUI::Server;

CCore::CCore()
  : Component(SERVER_CORE),
    DEFAULT_PATH("."),
    m_fileOpen(false),
    m_simRunning(false),
    m_active(false)
{
  m_commServer = new ServerNetworkComm();
  this->createSimulator("Simulator");

  connect(m_commServer, SIGNAL(newClient(int)),
          this,  SLOT(newClient(int)));

  regist_signal("readDir", "Read directory content")->connect(boost::bind(&CCore::readDir, this, _1));
  regist_signal("list_tree", "test")->connect(boost::bind(&CCore::list_tree, this, _1));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CCore::~CCore()
{
  delete m_commServer;
  delete m_srvSimulation;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CCore::listenToNetwork(const QString & hostname, quint16 portNumber)
{
  return m_commServer->openPort(hostname, portNumber);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::setHostList(const QList<HostInfos> & hostList)
{
  m_hostList = hostList;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<HostInfos> CCore::getHostList() const
{
  return m_hostList;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::sendSignal(const CF::Common::XmlDoc & signal)
{
  m_commServer->send(CFNULL, signal);
}

/***************************************************************************

PRIVATE METHODS

***************************************************************************/

void CCore::createSimulator(const QString & name)
{
  m_srvSimulation = new CSimulator(name);

  connect(m_srvSimulation, SIGNAL(message(const QString&)), this,
          SLOT(message(const QString&)));

  connect(m_srvSimulation, SIGNAL(error(const QString&)), this,
          SLOT(error(const QString&)));

  connect(m_srvSimulation, SIGNAL(finished()),
          this, SLOT(simulationFinished()));

  connect(m_commServer,
          SIGNAL(addComponent(const QString &,
                              CF::GUI::Network::ComponentType::Type,
                              const QString &)),
          m_srvSimulation,
          SLOT(addComponent(const QString &,
                            CF::GUI::Network::ComponentType::Type,
                            const QString &)));

  connect(m_srvSimulation, SIGNAL(treeUpdated()), this, SLOT(treeUpdated()));

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::setStatus(WorkerStatus::Type status)
{
  PEInterface::getInstance().change_status(status);
  //  this->commServer->sendStatus(-1, PE::getStatusString(status).c_str());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CCore::getDirContent(const QString & directory,
                          const QStringList & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          QStringList & dirsList,
                          QStringList & filesList) const
{
  QStringList list;
  QDir dir(directory);

  dir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
  dir.setSorting(QDir::DirsFirst | QDir::Name);

  if(!dir.exists())
    return false;

  QFileInfoList files = dir.entryInfoList();
  QFileInfoList::iterator it = files.begin();

  QRegExp regex("", Qt::CaseSensitive, QRegExp::RegExp);

  if(!extensions.isEmpty())
  {
    /* build the regex pattern string.
    For example, if the QStringList contains "xml" and "CFcase" extensions,
    the resulting string will be : "^.+\\.((xml)|(CFcase))$" */

    QString regexPattern = extensions.join(")|(");
    regexPattern.prepend("^.+\\.((").append("))$");
    regex.setPattern(regexPattern);
  }
  else
    regex.setPattern("^.+\\..+$");

  while(it != files.end())
  {
    QFileInfo fileInfo = *it;
    QString filename = fileInfo.fileName();

    if (filename != "." && filename != "..")
    {
      if(fileInfo.isDir())
        dirsList << filename;

      else if(includeFiles && regex.exactMatch(filename))
        filesList << filename;

      else if(includeFiles && includeNoExtension && !filename.contains('.'))
        filesList << filename;
    }
    it++;
  }

  return true;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                               BOOST SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Signal::return_t CCore::readDir(Signal::arg_t & node)
{
  XmlParams p(node);

  throw NotImplemented(FromHere(), "CCore::readDir");
//  QStringList dirList;
//  QStringList fileList;
//  QStringList extensions;
//  QString frame;
//  QString directory;
//
//  QString dirPath = p.value<std::string>("dirPath").c_str();
//  bool includeFiles = p.value<bool>("includeFiles");
//  bool includeNoExtension = p.value<bool>("includeNoExtension");
//
//  if(dirPath.isEmpty())
//    directory = this->DEFAULT_PATH;
//  else
//    directory = dirPath;
//
//  directory = QDir(directory).absolutePath();
//  directory = QDir::cleanPath(directory);
//
//  // if the directory is not the root
//  /// @todo test this on Windows
//  if(directory != "/")
//    dirList << "..";
//
//  SignalInfo::convertToStringList(p.array<std::string>("extensions"), extensions);
//
//  if(!this->getDirContent(dirPath, extensions, includeFiles,
//                          includeNoExtension, dirList, fileList))
//  {
//    m_commServer->sendError(-1, dirPath + ": no such direcrory");
//  }
//  else
//  {
//    // the reciever becomes the sender and vice versa
//    SignalInfo si("readDir", p.getReceiver(), p.getSender(), false);
//    QList<std::string> dirList2;
//    QList<std::string> fileList2;
//
//    SignalInfo::convertToStdString(dirList, dirList2);
//    SignalInfo::convertToStdString(fileList, fileList2);
//
//    // Build the reply
//
//    si.setParam("dirPath", directory.toStdString());
//    si.setArray("dirs", dirList2);
//    si.setArray("files", fileList2);
//
//    frame = si.getString();
//
//    m_commServer->send(-1, frame);
//  }

  /// @todo return something...
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CCore::createDir(Signal::arg_t & node)
{
  m_commServer->sendError(-1, "Cannot create a directory");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CCore::shutdown(Signal::arg_t & node)
{
  m_commServer->sendError(-1, "Cannot shudown");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CCore::saveConfig(Signal::arg_t & node)
{
  m_commServer->sendError(-1, "Cannot save the configuration");
}

Signal::return_t CCore::list_tree(Signal::arg_t & node)
{
  CFinfo << "Received signal!" << CFendl;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::treeUpdated()
{
  this->getTree(-1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::newClient(int clientId)
{
  if(m_fileOpen)
    m_commServer->sendAck(clientId, true, NETWORK_OPEN_FILE);

  if(m_simRunning)
    m_commServer->sendAck(clientId, true, NETWORK_SIMULATION_RUNNING);

  if(m_active)
    m_commServer->sendAck(clientId, true, NETWORK_ACTIVATE_SIMULATION);

  // send a welcome message to the new client
  m_commServer->sendMessage(clientId, "Welcome to the Client-Server project!");

  this->getTree(clientId);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::getTree(int clientId)
{
  QDomDocument d;
  d.setContent(m_srvSimulation->getTreeXML());
  m_commServer->sendTree(clientId, d);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::configureSimulator(int clientId, const QDomDocument & config)
{
  m_simTree = ConverterTools::xmlToXCFcase(config.toString().toStdString());
  //  ConfigArgs args = ConverterTools::xmlToConfigArgs(config.toString().toStdString());
  QString tree = m_simTree.createXMLString();
  QDomDocument document;

  document.setContent(tree);

  if(m_fileOpen)
    this->closeFile(clientId);

  m_srvSimulation->configureSimulator(document);

  m_fileOpen = true;
  m_commServer->sendAck(clientId, true, NETWORK_OPEN_FILE);

  // send the new tree to all client
  document.setContent(m_srvSimulation->getTreeXML());
  m_commServer->sendTree(-1, document);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::modifyNode(int clientId, const QDomDocument & data)
{
  if(!m_fileOpen)
    m_commServer->sendError(clientId, "No case file loaded !");

  else
    ; /// @todo forward to the simulator
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::deleteNode(int clientId, const QString & nodePath)
{
  if(!m_fileOpen)
    m_commServer->sendError(clientId, "No case file loaded !");

  else if(m_simRunning)
    m_commServer->sendError(clientId, "A simulation is running.");\

  else
    ; /// @todo forward to the simulator

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::addNode(int clientId, const QString & parentPath,
                           const QString & name, const QString & type,
                           const QString & absType)
{
  if(!m_fileOpen)
    m_commServer->sendError(clientId, "No case file loaded !");

  else if(m_simRunning)
    m_commServer->sendError(clientId, "A simulation is running.");

  else
    ; /// @todo forward to the simulator
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::renameNode(int clientId, const QString & nodePath,
                              const QString & newName)
{
  if(!m_fileOpen)
    m_commServer->sendError(clientId, "No case file loaded !");

  else if(m_simRunning)
    m_commServer->sendError(clientId, "A simulation is running.");

  else
    ; /// @todo forward to the simulator
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::getAbstractTypes(int clientId, const QString & typeName)
{
  QDomNode node = m_types.namedItem(typeName);
  QDomDocument document;
  QDomNodeList childNodes;
  QStringList typeList;

  // if the node is null, typeName is not a existing type man
  if(node.isNull())
  {
    m_commServer->sendError(clientId, QString("Type '%1' does not exist.")
                                  .arg(typeName));
    return;
  }

  childNodes = node.childNodes();

  // if no child, no types to send
  if(childNodes.isEmpty())
  {
    m_commServer->sendError(clientId, QString("No abstract type for type '%1'")
                                  .arg(typeName));
    return;
  }

  // building the types list
  for(int i = 0 ; i < childNodes.count() ; i++)
    typeList << childNodes.at(i).nodeName();

  m_commServer->sendAbstractTypes(clientId, typeName, typeList);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::getConcreteTypes(int clientId, const QString & typeName)
{
  QStringList typeList = m_srvSimulation->getConcreteTypes(typeName);
  m_commServer->sendConcreteTypes(clientId, typeName, typeList);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::createDirectory(int clientId, const QString & dirPath,
                                   const QString & name)
{
  /// @todo check if absolute
  /// @todo check if OS compatible
  QDir dir(dirPath + "/" + name);
  QString message = QString("Could not create '%1': ").arg(dir.absolutePath());

  // if a file or directory with that name already exists
  if(dir.exists())
  {
    message.append("a file or a directory with that name already exists");
    m_commServer->sendError(clientId, message);
  }

  // if directory creation failed
  else if(!dir.mkpath(dir.absolutePath()))
  {
    message.append("please check that you have the permission to create a "
                   "directory there.");
    m_commServer->sendError(clientId, message);
  }

  else
    m_commServer->sendAck(clientId, true, NETWORK_CREATE_DIR);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::openDirectory(int clientId, const QString & dirPath,
                                 const QStringList & extensions,
                                 bool includeFiles, bool includeNoExtension)
{
  QStringList directories;
  QStringList files;
  // bool dotDot;

  QString directory;

  QDomDocument filesList;

  if(dirPath.isEmpty())
    directory = this->DEFAULT_PATH;
  else
    directory = dirPath;

  directory = QDir(directory).absolutePath();
  directory = QDir::cleanPath(directory);

  if(directory != "/")
    directories << "..";

  if(!this->getDirContent(directory, extensions, includeFiles,
                          includeNoExtension, directories, files))
  {
    m_commServer->sendError(clientId, QString("'%1' is not an existing directory")
                                  .arg(directory));
    return;
  }

  m_commServer->sendDirContents(clientId, directory, directories, files);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::saveConfiguration(int clientId, const QString & filename,
                                     const QDomDocument & config)
{
  bool cfcase = false;
  QFile file(filename);
  QString configStr = config.toString();

  if(filename.endsWith(".CFcase"))
    cfcase = true;

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QString error = QString("Could open file '%1' for write access: %2")
    .arg(filename)
    .arg(file.errorString());

    m_commServer->sendError(clientId, error);
    m_commServer->sendAck(clientId, false, NETWORK_SAVE_CONFIG);
  }

  else
  {
    QTextStream out;

    out.setDevice(&file);

    if(cfcase)
    {
      std::string data = ConverterTools::xmlToCFcase(configStr.toStdString());
      out << data.c_str();
    }

    else
      out << configStr;

    file.close();
    m_commServer->sendAck(clientId, true, NETWORK_SAVE_CONFIG);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::openFile(int clientId, const QString & filename)
{
  if(m_fileOpen)
    this->closeFile(clientId);

  QDomDocument document;
  boost::filesystem::path file(filename.toStdString());

  //  ConfigFileReader configFile;
  ConfigArgs args;
  //configFile.parse(filename.toStdString(), args);

  m_simTree = XMLNode::parseString(ConverterTools::configArgsToXml(args).c_str());

  if(m_srvSimulation->loadCaseFile(filename))
  {
    // Notify all clients that a case file has been loaded
    m_commServer->sendAck(-1, true, NETWORK_OPEN_FILE);
    m_fileOpen = true;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::closeFile(int clientId)
{
  QString error;

  if(m_simRunning)
    error = "A simulation is running. You cannot close the file for now";
  else if(!m_fileOpen)
    error = "No file open.";
  if(!error.isEmpty())
    m_commServer->sendError(clientId, error);
  else
  {
    delete m_srvSimulation;
    this->createSimulator("Simulator");
    m_commServer->sendMessage(-1, "File closed");
    m_commServer->sendAck(-1, true, NETWORK_CLOSE_FILE);
    m_fileOpen = false;
  }

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::runSimulation(int clientId)
{
  QString error;

  if(!m_fileOpen)
    error = "Please open a case file or configure the simulator before running "
    "a simulation.";
  else if(m_simRunning)
    error = "The simulation is already running. You cannot run it twice at the "
    "same time.";
  else if(!m_active)
    error = "The simulation is not active.";

  if(!error.isEmpty())
    m_commServer->sendError(clientId, error);
  else
  {
    m_simulationManager.configure(m_simTree);

    //   this->simulationManager.start();

    m_simRunning = true;

    // Notify all clients that the simulation has started
    m_commServer->sendAck(-1, true, NETWORK_SIMULATION_RUNNING);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::shutdownServer(int clientId)
{
  qApp->exit(0); // exit the Qt event loop
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::message(const QString & message)
{
  m_commServer->sendMessage(-1, message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::error(const QString & message)
{
  m_commServer->sendError(-1, message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::simulationFinished()
{
  m_simRunning = false;
  m_commServer->sendAck(-1, true, NETWORK_RUN_SIMULATION);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::getHostList(int clientId)
{
  m_commServer->sendHostList(clientId, m_hostList);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::activateSimulation(int clientId, unsigned int nbProcs,
                                      const QString & hosts)
{
  QString error;

  if(!m_fileOpen)
    error = "Please open a case file or configure the simulator before activating "
    "the simulation.";
  else if(m_simRunning)
    error = "The simulation is already running.";
  else if(m_active)
    error = "The simulation is active.";

  if(!error.isEmpty())
  {
    m_commServer->sendError(clientId, error);
    m_commServer->sendAck(clientId, false, NETWORK_ACTIVATE_SIMULATION);
  }
  else
  {
    this->setStatus(WorkerStatus::STARTING);
    m_simulationManager.spawn("SubSystem", "SubSystem", nbProcs, hosts);
    m_active = true;
    m_simulationManager.run();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::deactivateSimulation(int clientId)
{
  QString error;

  if(!m_active)
    m_commServer->sendError(clientId, "The simulation is not active.");
  else
  {
    this->setStatus(WorkerStatus::EXITING);
    m_commServer->sendMessage(-1, "Exiting workers.");
    m_simulationManager.exitWorkers();
    m_commServer->sendAck(-1, true, NETWORK_DEACTIVATE_SIMULATION);
    CFinfo << "Simulation has been deativated.\n" << CFflush;
    m_active = false;
    this->setStatus(WorkerStatus::NOT_RUNNING);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::simulationStatus(const QString & subSysName, int rank,
                                    const QString & status)
{
  m_commServer->sendStatus(-1, subSysName, rank, status);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::getSubSysList(int clientId)
{
  QStringList list;

  int size = m_srvSimulation->readSubSystems();

  for(int i = 0 ; i < size ; i++)
    list << m_srvSimulation->getSubSystem(i).split(" ").at(0);

  m_commServer->sendSubSystemList(clientId, list);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::simulationTree(const XMLNode & tree)
{
  m_commServer->sendTree(-1, tree);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::spawned()
{
  m_commServer->sendAck(-1, true, NETWORK_ACTIVATE_SIMULATION);
  m_commServer->sendMessage(-1, "Simulation has been activated.");
}
