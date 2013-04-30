// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_server_ServerNetworkComm_hpp
#define cf3_ui_server_ServerNetworkComm_hpp

////////////////////////////////////////////////////////////////////////////////

//#include <QThread>

#include <boost/asio/ip/tcp.hpp> // for tcp::acceptor (nested classes cannot be forward declared)

#include "common/XML/XmlDoc.hpp"

#include "ui/uicommon/LogMessage.hpp"

//class QString;
//class QMutex;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

namespace network { class TCPConnection; class ErrorHandler; }

namespace server {

/////////////////////////////////////////////////////////////////////////////

/// @brief This class is the server network level.

/// For all signals emitted by this class, the first parameter identifies the
/// client that caused the signal to be emitted. A same client will always
/// have the same id. When client disconnects, its id will never be given
/// to another client.

/// @author Quentin Gasper.

class ServerNetworkComm :
//    public QThread,
    public common::SignalHandler
{

public: // nested structs

  struct ClientInfo
  {
  public:
    boost::shared_ptr<network::TCPConnection> connection;

    common::SignalArgs buffer;

    boost::shared_ptr<network::ErrorHandler> error_handler;

    std::string uuid;
  }; // ClientInfo

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
  bool open_port( unsigned short port = 62784 );

  void run();

  /// Stops all network operations
  void close();

  /// @brief Sends a message to a client

  /// @param message Message to send
  /// @param uuid Client UuiD, or an empty string to send to all clients.
  /// @throw UnknownClientIdException if Client UuiD is unknown.
  void send_message_to_client( const std::string & message,
                               uiCommon::LogMessage::Type type,
                               const std::string & uuid = std::string() );

  void send_frame_to_client( common::XML::SignalFrame & signal,
                             const std::string & uuid = std::string() );

  void send_frame_rejected_to_client( const std::string clientid,
                                      const std::string & frameid,
                                      const common::URI & sender,
                                      const std::string & reason );


private: // functions

  void init_accept();

  void init_send( boost::shared_ptr<network::TCPConnection> client,
                  common::XML::SignalFrame & frame );

  void init_read( ClientInfo & client );

  void callback_accept( boost::shared_ptr<network::TCPConnection> conn,
                        const boost::system::error_code & error );

  void callback_send( boost::shared_ptr<network::TCPConnection> conn,
                      const boost::system::error_code & error );

  void callback_read( boost::shared_ptr<network::TCPConnection> conn,
                      const boost::system::error_code & error );


private:


  boost::asio::io_service * m_io_service;

  boost::asio::ip::tcp::acceptor * m_acceptor;

  unsigned short m_port;

  /// @brief Mutex for thread-safe operations.
//  QMutex * m_mutex;

  /// @brief The client sockets.

  /// The key is pointer to the m_socket. The value is the client UuiD.
  std::map<boost::shared_ptr<network::TCPConnection>, ClientInfo> m_clients;

  /// @brief Sends a message to a client.

  /// @param client Client socket to use. If @c nullptr, the message will be
  /// sent to all clients.
  /// @param message Message to send.
  /// @return Returns @c true if the frame was built and sent successfully;
  /// otherwise returns @c false.
  void send_message( boost::shared_ptr<network::TCPConnection> client,
                     const std::string & message,
                     uiCommon::LogMessage::Type type );

  /// @brief Sends a message to a client.

  /// @param client Client m_socket to use. If @c nullptr, the frame will be sent
  /// to all clients.
  /// @param signal Signal frame to send.
  /// @return Returns the number of bytes sent.
  void send( boost::shared_ptr<network::TCPConnection> client,
             const common::XML::XmlDoc & signal );

  void send_frame_rejected( boost::shared_ptr<network::TCPConnection> client,
                            const std::string & frameid,
                            const cf3::common::URI & sender,
                            const std::string & reason );

  /// @brief Retrieves a client socket from its UuiD.

  /// @param uuid Client UuiD
  /// @return Returns a pointer to the socket, or @c nullptr if client
  /// UuiD was -1 (all clients).
  /// @throw UnknownClientIdException if Client id is unknown.
  boost::shared_ptr<network::TCPConnection> get_connection(const std::string & uuid) const;

}; // class ServerNetworkComm

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_server_ServerNetworkComm_hpp
