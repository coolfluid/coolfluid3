#include <QtXml>
#include <QHostInfo>
#include <mpi.h>

#include <boost/filesystem/path.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/MPI/PEInterface.hpp"
#include "Common/Log.hpp"
#include "Common/ConfigArgs.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/HostInfos.hpp"

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
  BUILD_COMPONENT;

  TypeInfo::instance().regist<CCore>( type_name() );

  m_commServer = new ServerNetworkComm();
  this->createSimulator("Simulator");

  connect(m_commServer, SIGNAL(newClient(int)),
          this,  SLOT(newClient(int)));

  regist_signal("read_dir", "Read directory content")->connect(boost::bind(&CCore::read_dir, this, _1));
  regist_signal("shutdown", "Shutdown the server")->connect(boost::bind(&CCore::shutdown, this, _1));
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
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::setStatus(WorkerStatus::Type status)
{
  PEInterface::instance().change_status(status);
  //  this->commServer->sendStatus(-1, PE::getStatusString(status).c_str());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CCore::getDirContent(const QString & directory,
                          const std::vector<std::string> & extensions,
                          bool includeFiles,
                          bool includeNoExtension,
                          std::vector<std::string> & dirsList,
                          std::vector<std::string> & filesList) const
{
  QStringList list;
  QDir dir(directory);
  bool dirExists = dir.exists();

  dir.setFilter(QDir::Files | QDir::Dirs | QDir::Hidden);
  dir.setSorting(QDir::DirsFirst | QDir::Name);

  if(dirExists)
  {
    QFileInfoList files = dir.entryInfoList();
    QFileInfoList::iterator it = files.begin();

    QRegExp regex("", Qt::CaseSensitive, QRegExp::RegExp);

    if(!extensions.empty())
    {
      /* build the regex pattern string.
    For example, if the QStringList contains "xml" and "CFcase" extensions,
    the resulting string will be : "^.+\\.((xml)|(CFcase))$" */

      /// @todo try to use QString::resize() or QString::reserve()
      QString regexPattern;
      std::vector<std::string>::const_iterator it = extensions.begin();

      while(it != extensions.end())
      {
        if(!regexPattern.isEmpty())
          regexPattern.append(")|(");

        regexPattern.append(it->c_str());

        it++;
      }

//      QString regexPattern = extensions.join(")|(");
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
          dirsList.push_back(filename.toStdString());
        else if(includeFiles)
        {
          if(regex.exactMatch(filename))
            filesList.push_back(filename.toStdString());
          else if(includeNoExtension && !filename.contains('.'))
            filesList.push_back(filename.toStdString());
        }
      }

      it++;
    }
  }

  return dirExists;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                               BOOST SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

Signal::return_t CCore::read_dir(Signal::arg_t & node)
{
  XmlParams p(node);
  std::vector<std::string> dirList;
  std::vector<std::string> fileList;
  QString directory;

  try
  {
    QString dirPath = p.get_param<std::string>("dirPath").c_str();
    bool includeFiles = p.get_param<bool>("includeFiles");
    bool includeNoExtension = p.get_param<bool>("includeNoExtensions");
    std::vector<std::string> extensions = p.get_array<std::string>("extensions");

    if(dirPath.isEmpty())
      directory = this->DEFAULT_PATH;
    else
      directory = dirPath;

    directory = QDir(directory).absolutePath();
    directory = QDir::cleanPath(directory);

    // if the directory is not the root
    /// @todo test this on Windows
    if(directory != "/")
      dirList.push_back("..");

    if(!this->getDirContent(directory, extensions, includeFiles,
                            includeNoExtension, dirList, fileList))
    {
      m_commServer->sendError(-1, dirPath + ": no such direcrory");
    }
    else
    {
      // Build the reply
      XmlNode * replyNode = XmlOps::add_reply_frame(node);
      XmlParams reply(*replyNode);

      reply.add_param("dirPath", directory.toStdString());
      reply.add_array("dirs", dirList);
      reply.add_array("files", fileList);
    }
  }
  catch(Exception e)
  {
    CFerror << e.what() << CFflush;
  }

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
  qApp->exit(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CCore::saveConfig(Signal::arg_t & node)
{
  m_commServer->sendError(-1, "Cannot save the configuration");
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                 SLOTS

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void CCore::newClient(int clientId)
{
  // send a welcome message to the new client
  m_commServer->sendMessage(clientId, "Welcome to the Client-Server project!");
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
  throw NotImplemented(FromHere(), "CCore::simulationFinished");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::activateSimulation(int clientId, unsigned int nbProcs,
                                      const QString & hosts)
{
  throw NotImplemented(FromHere(), "CCore::activateSimulation");

//  QString error;

//  if(!m_fileOpen)
//    error = "Please open a case file or configure the simulator before activating "
//    "the simulation.";
//  else if(m_simRunning)
//    error = "The simulation is already running.";
//  else if(m_active)
//    error = "The simulation is active.";

//  if(!error.isEmpty())
//  {
//    m_commServer->sendError(clientId, error);
//    m_commServer->sendAck(clientId, false, NETWORK_ACTIVATE_SIMULATION);
//  }
//  else
//  {
//    this->setStatus(WorkerStatus::STARTING);
//    m_simulationManager.spawn("SubSystem", "SubSystem", nbProcs, hosts);
//    m_active = true;
//    m_simulationManager.run();
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::deactivateSimulation(int clientId)
{
  throw NotImplemented(FromHere(), "CCore::deactivateSimulation");

//  QString error;

//  if(!m_active)
//    m_commServer->sendError(clientId, "The simulation is not active.");
//  else
//  {
//    this->setStatus(WorkerStatus::EXITING);
//    m_commServer->sendMessage(-1, "Exiting workers.");
//    m_simulationManager.exitWorkers();
//    m_commServer->sendAck(-1, true, NETWORK_DEACTIVATE_SIMULATION);
//    CFinfo << "Simulation has been deativated.\n" << CFflush;
//    m_active = false;
//    this->setStatus(WorkerStatus::NOT_RUNNING);
//  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CCore::spawned()
{
  throw NotImplemented(FromHere(), "CCore::spawned");
}
