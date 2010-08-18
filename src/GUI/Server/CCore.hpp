#ifndef CF_GUI_Server_CCore_h
#define CF_GUI_Server_CCore_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QThread>
#include <QList>
#include <QDomDocument>
#include <mpi.h>
#include <vector>
#include <string>

#include "Common/Component.hpp"
#include "Common/MPI/PEInterface.hpp"

#include "GUI/Network/HostInfos.hpp"
#include "GUI/Server/CSimulator.hpp"
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

     void setHostList(const QList<CF::GUI::Network::HostInfos> & hostList);

     QList<CF::GUI::Network::HostInfos> getHostList() const;

     void sendSignal(const CF::Common::XmlDoc & signal);

     static std::string type_name() { return "CCore"; }

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

  private:
    /// @brief The default path for the file browsing.

    /// The default path is the current directory (./).
    const QString DEFAULT_PATH;

    /// @brief The simulation.
    CSimulator * m_srvSimulation;

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

    /// @brief Host list
    QList<CF::GUI::Network::HostInfos> m_hostList;

    /// @brief Creates a new simulator with the name @c name.

    /// If a simulator already exists, it is not destroyed. If a file is open,
    /// it is not closed.
    /// @param name Name of the new simulator.
    void createSimulator(const QString & name);

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

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

    CF::Common::Signal::return_t read_dir(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t createDir(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t shutdown(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t saveConfig(CF::Common::Signal::arg_t & node);

    CF::Common::Signal::return_t list_tree(CF::Common::Signal::arg_t & node);
  };

////////////////////////////////////////////////////////////////////////////

} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Server_CCore_h
