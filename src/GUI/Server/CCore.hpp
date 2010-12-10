// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Server_CCore_h
#define CF_GUI_Server_CCore_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QThread>
#include <QList>
#include <QDomDocument>
#include <vector>
#include <string>

#include "Common/Log.hpp"       // temporary
#include "Common/Component.hpp"
#include "Common/MPI/PE.hpp"

#include "GUI/Server/SimulationManager.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

////////////////////////////////////////////////////////////////////////////

  class ServerNetworkComm;
  class CSimulator;

  /// @brief Manages everything that is not related to the network layer
  /// or the simulation management.

  /// @author Quentin Gasper

  class CCore :
      public QObject,
      public CF::Common::Component
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<CCore> Ptr;
    typedef boost::shared_ptr<CCore const> ConstPtr;

    /// @brief Constructor

    CCore();

    /// @brief Destructor

    /// Free all allocated memory.
     ~CCore();

     /// @param hostame Host name
     /// @param portNumber Port number
     /// @throw NetworkException
     bool listenToNetwork(const QString & hostname, quint16 portNumber);

     void sendSignal(const CF::Common::XmlNode & signal);

     static std::string type_name() { return "CCore"; }

     void sendFrameRejected(const std::string & clientid,
                            const std::string & frameid,
                            const CF::Common::URI & sender,
                            const QString & reason);

     void sendException(const char * what,
                        const std::string & clientid = std::string());

  private slots:

    /// @brief Slot called when a new client connects

    /// Sends server status (file open, simulation running) to the new client.
    /// @param clientId New client id.
     void newClient(const std::string & uuid);

    /// @brief Forwards a message from the simulator to the network layer
    /// @param message Message to send
    void message(const QString & message);

    /// @brief Forwards an error message from the simulator to the network layer
    /// @param message Error message to send
    void error(const QString & message);

    /// @brief Slot called when the simulation is done.
    void simulationFinished();

    /// @brief Slot called when the client wants to activate the simulation

    /// @param clientId Client ID.
    /// @param nbProcs Number of workers to spawn
    /// @param hosts Hosts on which the workers will be spawned.
    void activateSimulation(int clientId, unsigned int nbProcs,
                            const QString & hosts);

    /// @brief Slot called when the client wants to deactivate the simulation
    /// @param clientId Client ID.
    void deactivateSimulation(int clientId);

    void spawned();

  public slots:

    void newEvent(const std::string & name, const CF::Common::URI & path);

  private:
    /// @brief The default path for the file browsing.

    /// The default path is the current directory (./).
    const QString DEFAULT_PATH;

    /// @brief The network communication
    ServerNetworkComm * m_commServer;

    /// @brief XML tree with available types, their concrete and abstract
    /// sub-types.

    /// The content of this tree is read from "./TypesList.xml" file
    QDomDocument m_types;

    /// @brief Indicates wether a file is already open.

    /// If @c true, a file is already open.
    bool m_fileOpen;

    /// @brief Indicates wether the simulation is running.

    /// If @c true, the simulation is running.
    bool m_simRunning;

    bool m_active;

    /// @brief Simulation manager
    SimulationManager m_simulationManager;

    /// @brief Reads a directory contents.

    /// @param directory Directory to read.
    /// @param extensions List of wanted extensions. According to the network
    /// m_protocol, this list may be empty.
    /// @param includeFiles If @c true, sub-directories and files returned.
    /// If @c false, only sub-directories will be returned.
    /// @param includeNoExtension If @c true, files without any extension will
    /// be returned. If @c false, they will not.
    /// @param dirsList Reference of a @c QStringList where sub-directories
    /// names will be stored.
    /// @param filesList Reference of a @c QStringList where files names will
    /// be stored.
    /// @return Returns @c true if the directory has been correctly read.
    /// Otherwise, returns @c false (@c dirsList and @c filesList are not
    /// modified in this case).
    bool getDirContent(const QString & directory,
                       const std::vector<std::string> & extensions,
                       bool includeFiles,
                       bool includeNoExtension,
                       std::vector<std::string> & dirsList,
                       std::vector<std::string> & filesList) const;

    void setStatus(CF::Common::WorkerStatus::Type status);

    CF::Common::Signal::return_t read_dir(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t createDir(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t shutdown(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t saveConfig(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t list_tree(CF::Common::Signal::arg_t & node);

    /// @todo this is a signal to test server business, queuing,...
    /// should be removed ASAP
    CF::Common::Signal::return_t heavy_compute(CF::Common::Signal::arg_t & node)
    {
      CFinfo << "Waiting..." << CFendl;

      for(int i = 0 ; i < 10 ; i++)
      {
        std::system("sleep 1");
      }

      CFinfo << "...done!" << CFendl;
    }
  };

////////////////////////////////////////////////////////////////////////////

} // Server
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_CCore_h
