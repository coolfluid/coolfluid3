#ifndef COOLFluid_server_ServerNetworkComm_h
#define COOLFluid_server_ServerNetworkComm_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QAbstractSocket>
#include <QDomDocument>
#include <QList>
#include <QMutex>

#include "GUI/Network/HostInfos.hpp"
#include "GUI/Network/ComponentType.hpp"
#include "GUI/Network/SignalInfo.hpp"

class QHostAdress;
class QTcpServer;
class QTcpSocket;
class QString;

///////////////////////////////////////////////////////////////////////////////

#include "Common/XmlHelpers.hpp"

namespace CF {

namespace GUI {

namespace Network
{
  class ComponentType;
  struct FrameInfos;
}

namespace Server {

/////////////////////////////////////////////////////////////////////////////

  class CSimulator;

  /// @brief This class is the server network level.

  /// For all signals emitted by this class, the first parameter identifies the
  /// client that caused the signal to be emitted. A same client will always
  /// have the same id. When client disconnects, its id will never be given
  /// to another client.

  /// @author Quentin Gasper.
  class ServerNetworkComm : public QObject
  {
    Q_OBJECT

  public:

    /// @brief Constructor.

    ServerNetworkComm();

    /// @brief Destructor.

    /// Closes the sockets before the object is deleted.
    ~ServerNetworkComm();

    /// @brief Opens a port the server has to listen to.

    /// @param hostAddress Server address.
    /// @param port Socket port.
    /// @throw NetworkException Throws a NetworkException if the server cannot
    /// listen to the given address/port.
    bool openPort(const QString & hostAddress = "127.0.0.1", quint16 port = 62784);

    /// @brief Gives the number of bytes recieved.

    /// @return Returns the number of bytes recieved.
    int getBytesRecieved() const;

    /// @brief Gives the number of bytes sent.

    /// @return Returns the number of bytes sent.
    int getBytesSent() const;

    /// @brief Sends an error message to a client

    /// @param clientId Client id, or -1 to send to all clients.
    /// @param message Message to send
    /// @throw UnknownClientIdException if Client id is unknown.
    void sendError(int clientId, const QString & message);

    /// @brief Sends a message to a client

    /// @param clientId Client id, or -1 to send to all clients.
    /// @param message Message to send
    /// @throw UnknownClientIdException if Client id is unknown.
    void sendMessage(int clientId, const QString & message);

    void send(int clientId, const CF::Common::XmlNode & signal);

  private slots :

    /// @brief Slot called when a new client tries to connect.
    void newClient();

    /// @brief Slot called when new data are available on one of the
    /// client sockets.
    void newData();

    /// @brief Slot called when the client has been disconnected.
    void clientDisconnected();

    /// @brief Slot called when a message (i.e. output forwarding) comes
    /// from the simulator.

    /// Sends this message to all clients.
    /// @param message The message.
    void message(const QString & message);

    /// @brief Slot called when an error comes from the simulator.

    /// Sends this error message to all clients.
    /// @param message The error message.
    void error(const QString & message);

  signals:

    void newClient(int clientId);

  private:

    /// @brief The server m_socket.

    /// Used to accept connections.
    QTcpServer * m_server;

    /// @brief The server m_socket for the local loop.

    /// Used to accept connections coming from "localhost" (local loop).
    QTcpServer * m_localSocket;

    /// @brief Size of the frame that is being read.

    /// If the value is 0, no frame is currently being recieved.
    quint32 m_blockSize;

    /// @brief The client sockets.

    /// The key is pointer to the m_socket. ...
    QHash<QTcpSocket *, QDomNode> m_clients;

    /// @brief Number of bytes recieved.
    int m_bytesRecieved;

    /// @brief Number of bytes sent.
    int m_bytesSent;

    /// @brief Last ID given to a client
    int m_lastClientId;

    /// @brief Hash map that associated an ID to a client m_socket

    /// The key is the client ID. The value is the client m_socket.
    QHash<int, QTcpSocket *> m_clientIds;

    /// @brief Sends a message to a client.

    /// @param client Client m_socket to use. If @c CFNULL, the message will be
    /// sent to all clients.
    /// @param message Message to send.
    /// @return Returns @c true if the frame was built and sent successfully;
    /// otherwise returns @c false.
    bool sendMessage(QTcpSocket * client, const QString & message);

    /// @brief Sends an error message to a client.

    /// @param client Client m_socket to use. If @c CFNULL, the error message will
    /// be sent to all clients.
    /// @param message Error message to send.
    void sendError(QTcpSocket * client, const QString & message);

    /// @brief Sends a message to a client.

    /// @param client Client m_socket to use. If @c CFNULL, the frame will be sent
    /// to all clients.
    /// @param frame Frame to send.
    /// @return Returns the number of bytes sent.
    int send(QTcpSocket * client, const QString & frame);

    /// @brief Retrieves a client socket from its id.

    /// @param clientId Client id
    /// @return Returns a pointer to the m_socket, or @c CFNULL if client id was
    /// -1 (all clients).
    /// @throw UnknownClientIdException if Client id is unknown.
    QTcpSocket * getSocket(int clientId) const;

  }; // class ServerNetworkComm

////////////////////////////////////////////////////////////////////////////

} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // COOLFluid_server_ServerNetworkComm_h
