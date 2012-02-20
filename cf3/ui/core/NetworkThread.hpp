// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_NetworkThread_hpp
#define cf3_ui_core_NetworkThread_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QThread>
#include <QMutex>

#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include "common/SignalHandler.hpp"
#include "common/XML/SignalFrame.hpp"

#include "ui/core/LibCore.hpp"

namespace boost { namespace asio { class io_service; } }

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace common { namespace XML { class XmlDoc; } }

namespace ui {

namespace network { class TCPConnection; class ErrorHandler; }

namespace core {

////////////////////////////////////////////////////////////////////////////////

/// Thread that manages the network.
///
/// @author Quentin Gasper.

class Core_API NetworkThread :
    public QThread,
    public common::SignalHandler
{

public:

  /// Constructor.
  NetworkThread(QObject * parent = nullptr);

  ~NetworkThread();

  /// Attempts to connect to a remote host.
  /// @warning This is a non-blocking method. It does not wait the connection
  /// to be established. It sends a request to the system and returns directly
  /// after.
  /// @param hostAddress Address of the host to connect to.
  /// @param port The port number to use.
  bool connect_to_host( const std::string & hostAddress, unsigned short port );

  /// @brief Disconnects from the server, then closes.

  /// @param shutServer If @c true, a request to shut down the server is sent.
  void disconnect_from_server( bool shutServer );

  bool is_connected() const;

  /// @brief Sends a signal frame to the server.
  /// The client UuiD is added to the frame.
  /// @param signal The signal to send.
  void send( common::SignalArgs & signal );

  /// Executes the thread event loop.
  void run();

  void set_error_handler( boost::weak_ptr<network::ErrorHandler> handler );


private: // callback functions

  void init_read();

  void callback_connect( const boost::system::error_code & error );

  void callback_read( const boost::system::error_code & error );

  void callback_send( const boost::system::error_code & error );

private: // data


  boost::shared_ptr<network::TCPConnection> m_connection;

  common::SignalArgs m_buffer;

  boost::asio::io_service * m_io_service;

  boost::asio::ip::tcp::endpoint * m_endpoint;

  boost::shared_ptr<network::ErrorHandler> m_error_handler;

  bool m_request_disc;

  QString m_hostname;

  quint16 m_port;

  QMutex m_mutex;

}; // NetworkThread

////////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // NetworkThread_HPP
