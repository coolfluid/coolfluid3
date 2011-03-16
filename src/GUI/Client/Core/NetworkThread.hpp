// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NetworkThread_hpp
#define CF_GUI_Client_Core_NetworkThread_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QAbstractSocket>
#include <QThread>

#include "Common/SignalHandler.hpp"

#include "GUI/Client/Core/LibClientCore.hpp"

class QTcpSocket;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

/// Thread that manages the network.
///
/// @author Quentin Gasper.

class ClientCore_API NetworkThread : public QThread
{
  Q_OBJECT

public:

  /// Constructor.
  /// @param parent The thread parent. May be null.
  NetworkThread(QObject *parent = 0);

  /// Attempts to connect to a remote host.
  /// @warning This is a non-blocking method. It does not wait the connection
  /// to be established. It sends a request to the system and returns directly
  /// after.
  /// @param hostAddress Address of the host to connect to.
  /// @param port The port number to use.
  bool connectToHost(const QHostAddress& hostAddress, quint16 port);

  /// @brief Disconnects from the server, then closes.

  /// @param shutServer If @c true, a request to shut down the server is sent.
  void disconnect(bool shutServer);

  bool isConnected() const;

  /// @brief Sends a signal frame to the server.
  /// The client UUID is added to the frame.
  /// @param signal The signal to send.
  /// @return Returns the number of bytes written.
  int send(Common::Signal::arg_t & signal);

  /// Executes the thread event loop.
  void run();

public: // boost signals

  /// Signal executed when a new frame arrived (given as parameter).
  boost::signals2::signal< void (Common::XML::XmlDoc::Ptr) > newSignal;

private slots :

  /// @brief Slot called when there is an error on the socket.
  void newData();

  /// @brief Slot called when the connection has been broken or closed.
  /// If the thread is running, it is exited with error code 0 if
  /// @c #disconnect() was called or 1 if the disconnection is due to an
  /// error code.
  void disconnected();

  /// @brief Slot called when there is an error on the m_socket.

  /// @param err Error that occured.
  void socketError(QAbstractSocket::SocketError err);

signals:

  /// The signal is not emitted if the user resquested a disconnection (if
  /// @c #m_requestDisc is @c true ).
  void disconnectedFromServer();

  /// @brief Signal emitted when a connection has been successfully
  /// established between the client and the server.

  /// The signal is emitted exactly once when the first frame is
  /// recieved from the server.
  void connected();

private: // data

  QTcpSocket * m_socket;

  QString m_failureReason;

  quint32 m_blockSize;

  bool m_success;

  bool m_requestDisc;

}; // NetworkThread

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // NetworkThread_HPP
