#ifndef CF_GUI_Client_ClientCore_hpp
#define CF_GUI_Client_ClientCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QMap>
#include <QObject>

#include "Common/Component.hpp"
#include "Common/SignalHandler.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/TreeModel.hpp"
#include "GUI/Client/TSshInformation.hpp"

#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/NetworkFrameType.hpp"
#include "GUI/Network/HostInfos.hpp"

#include "GUI/Network/SignalInfo.hpp"

class QProcess;
class QString;
class QTimer;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  class ClientNetworkComm;
  class StatusModel;
  struct TSshInformation;

////////////////////////////////////////////////////////////////////////////////

  class ClientCore :
      public QObject,
      public CF::Common::Component
  {
    Q_OBJECT

    public:

    static ClientCore & getInstance();

    void setTreeModel(TreeModel * treeModel);

    void setStatusModel(StatusModel * statusModel);

    TreeModel * getTreeModel() const;

    StatusModel * getStatusModel() const;

    void sendSignal(const CF::GUI::Network::SignalInfo & signal);

    void buildAndSendSignal(const QString & type,
                            const CF::Common::CPath & sender,
                            const CF::Common::CPath & receiver);

    void sendSignal(const CF::Common::XmlDoc & signal);

  private slots:

    /// @brief Slot called when the client is connected to the server.
    void connected();

    /// @brief Tries to connect to the server.

    /// During the waiting for the server to launch through an SSH
    /// connection, this slot is called at every timeout of
    /// @c #timer and tries to connect to the server.
    void tryToConnect();

    void sshError();

  public slots:

    /// @brief Slot called when user wants to connect a simulation to its server.

    /// @param index Simulation index
    /// @param info Connection information
    /// @todo Replace QModelIndex by QPersistentModelIndex
    void connectSimulation(const QModelIndex & index,
                           const TSshInformation & info);

    signals:

    /// @brief Signal emitted when the server sends a directory contents.

    /// @param path Absolute path of the directoy of which contents belong to.
    /// @param dirs Directories list. Each element is a directory.
    /// @param files Files list. Each element is a file.
    void dirContents(const QString & path, const QStringList & dirs,
                     const QStringList & files);

    void acked(CF::GUI::Network::NetworkFrameType type);

    private: // methods

    ClientCore();

    ~ClientCore();

    void connectToServer(const QModelIndex & simIndex);

    void launchServer(const QModelIndex & simIndex);

  private: // data

    TreeModel * m_treeModel;

    StatusModel * m_statusModel;

    ClientNetworkComm * m_networkComm;

    QTimer * m_timer;

    QProcess * m_process;

    TSshInformation m_commSshInfo;

  }; // class ClientCore

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_ClientCore_hpp
