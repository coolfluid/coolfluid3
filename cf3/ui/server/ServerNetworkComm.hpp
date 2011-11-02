// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef COOLFluid_server_ServerNetworkComm_h
#define COOLFluid_server_ServerNetworkComm_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QAbstractSocket>
#include <QList>
#include <QMutex>

#include "common/XML/XmlDoc.hpp"

#include "ui/uicommon/LogMessage.hpp"

class QHostAdress;
class QTcpServer;
class QTcpSocket;
class QString;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace Server {

/////////////////////////////////////////////////////////////////////////////

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

    /// All network interfaces (local loop, ethernet, wi-fi,...) are listened to.
    /// @param hostAddress Server address.
    /// @throw NetworkException Throws a NetworkException if the server cannot
    /// listen to the given port.
    bool openPort(quint16 port = 62784);

    /// @brief Gives the number of bytes recieved.

    /// @return Returns the number of bytes recieved.
    int getBytesRecieved() const;

    /// @brief Gives the number of bytes sent.

    /// @return Returns the number of bytes sent.
    int getBytesSent() const;

    /// @brief Sends a message to a client

    /// @param message Message to send
    /// @param uuid Client UuiD, or an empty string to send to all clients.
    /// @throw UnknownClientIdException if Client UuiD is unknown.
    void sendMessageToClient(const QString & message,
                             uiCommon::LogMessage::Type type,
                             const std::string & uuid = std::string());

    void sendSignalToClient(const common::XML::XmlDoc & signal,
                            const std::string & uuid = std::string());

    void sendFrameRejectedToClient(const std::string clientid,
                                   const std::string & frameid,
                                   const common::URI & sender,
                                   const QString & reason);

    void disconnectAll();


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

    void newClientConnected(const std::string & uuid);

  private:

    /// @brief The server m_socket.

    /// Used to accept connections.
    QTcpServer * m_server;

    /// @brief Size of the frame that is being read.

    /// If the value is 0, no frame is currently being recieved.
    quint32 m_blockSize;

    /// @brief The client sockets.

    /// The key is pointer to the m_socket. The value is the client UuiD.
    QHash<QTcpSocket *, std::string> m_clients;

    /// @brief Number of bytes recieved.
    int m_bytesRecieved;

    /// @brief Number of bytes sent.
    int m_bytesSent;

    /// @brief Last ID given to a client
    int m_lastClientId;

    /// @brief Sends a message to a client.

    /// @param client Client socket to use. If @c nullptr, the message will be
    /// sent to all clients.
    /// @param message Message to send.
    /// @return Returns @c true if the frame was built and sent successfully;
    /// otherwise returns @c false.
    bool sendMessage(QTcpSocket * client, const QString & message,
                     uiCommon::LogMessage::Type type);

    /// @brief Sends a message to a client.

    /// @param client Client m_socket to use. If @c nullptr, the frame will be sent
    /// to all clients.
    /// @param signal Signal frame to send.
    /// @return Returns the number of bytes sent.
    int send(QTcpSocket * client, const common::XML::XmlDoc & signal);

    bool sendFrameRejected(QTcpSocket * client,
                           const std::string & frameid,
                           const cf3::common::URI & sender,
                           const QString & reason);

    /// @brief Retrieves a client socket from its UuiD.

    /// @param uuid Client UuiD
    /// @return Returns a pointer to the socket, or @c nullptr if client
    /// UuiD was -1 (all clients).
    /// @throw UnknownClientIdException if Client id is unknown.
    QTcpSocket * getSocket(const std::string & uuid) const;

    std::string getAttr(const common::XML::XmlNode & node,
                        const char * paramName,
                        QString & reason);

  }; // class ServerNetworkComm

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // COOLFluid_server_ServerNetworkComm_h
