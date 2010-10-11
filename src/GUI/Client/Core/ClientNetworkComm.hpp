// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_ClientNetworkComm_h
#define CF_GUI_Client_Core_ClientNetworkComm_h

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QAbstractSocket>

#include "Common/XmlHelpers.hpp"

#include "GUI/Network/ComponentType.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

class QString;
class QTcpSocket;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace Network {
  class SignalInfo;
  class NetworkProtocol;
  class NetworkException;
  struct HostInfos;
}

namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class represents the client network level.

  /// It operates mainly using Qt slots/signals system. Each time a frame
  /// arrives through the socket, the appropriate signal is thrown. Frames
  /// to send are built using the BuilderParser system.

  /// @author Quentin Gasper.

  class ClientCore_API ClientNetworkComm : public QObject
  {
    Q_OBJECT

  public:

    /// @brief Constructor.

    /// The socket @c #commClient is set to @c nullptr.
    ClientNetworkComm();

    /// @brief Destructor.

    /// Closes the sockets and free all allocated memory before the object
    /// is deleted.
    ~ClientNetworkComm();

    /// @brief Attempts to connect the client to the server.

    /// When this method returns, the m_socket is not open yet. The signal
    /// @c connected() will be emitted when the first frame
    /// arrives.
    /// @param hostAddress Server address.
    /// @param port Socket port number.
    /// @param m_skipRefused Value of @c #m_skipRefused during the attempt.
    void connectToServer(const QString & hostAddress = "127.0.0.1",
                         quint16 port = 62784, bool skipRefused = false);

    /// @brief Disconnects from the server, then closes.

    /// After calling this method, @c #m_requestDisc is @c true.
    /// @param shutServer If @c true, a request to shut down the server is sent.
    void disconnectFromServer(bool shutServer);

    /// @brief Indicates wether a connection is established to a server.

    /// @return Returns @c true if the client is connected to a server;
    /// otherwise, returns @c false.
    bool isConnected() const;

    bool send(CF::Common::XmlDoc & signal);

  private slots :

    /// @brief Slot called when there is an error on the socket.
    void newData();

    /// @brief Slot called when the connection has been successfully established.
    void connectionEstablished();

    /// @brief Slot called when the connection has been broken.
    void disconnected();

    /// @brief Slot called when there is an error on the m_socket.

    /// @param err Error that occured.
    void socketError(QAbstractSocket::SocketError err);

  signals:

    /// The signal is not emitted if the user resquested a disconnection (if
    /// @c #m_requestDisc is @c true ).
    void disconnectFromServer();

    /// @brief Signal emitted when a connection has been successfully
    /// established between the client and the server.

    /// The signal is emitted exactly once when the first frame is
    /// recieved from the server.
    void connected();

  private: // methods

    /// @brief Sends a frame to the server.

    /// All @e sendXXX methods of this class call this method to
    /// send their frames.
    /// @param frame Frame to send.
    int send(const QString & frame) const;

    bool checkConnected();

  private: // data

    /// @brief Socket used to communicate with the server.
    QTcpSocket * m_socket;

    /// @brief Size of the frame that is being read.

    /// If the value is 0, no frame is currently being recieved.
    quint32 m_blockSize;

    /// @brief Indicates wether the upper level requested a disconnection.
    bool m_requestDisc;

    /// @brief Indicates wether the m_socket is open and connected to the
    /// server.
    bool m_connectedToServer;

    /// @brief Indicates wether a "Connection refused" error must be skip.

    /// If @c true when a "Connection refused" error occurs, it is
    /// skipped and this attribute is set to @c false.
    bool m_skipRefused;

  }; // class ClientNetworkComm

////////////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_ClientNetworkComm_h
