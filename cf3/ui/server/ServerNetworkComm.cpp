// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "rapidxml/rapidxml.hpp"

#include "common/OptionT.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalFrame.hpp"

#include "common/XML/FileOperations.hpp"
#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "ui/uicommon/ComponentNames.hpp"
#include "ui/uicommon/LogMessage.hpp"

#include "ui/network/ErrorHandler.hpp"
#include "ui/network/TCPConnection.hpp"

#include "ui/server/ServerExceptions.hpp"
#include "ui/server/ServerRoot.hpp"

#include "ui/server/ServerNetworkComm.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common;
using namespace cf3::common::XML;

using namespace cf3::ui::network;
using namespace cf3::ui::server;
using namespace cf3::ui::uiCommon;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace server {

/////////////////////////////////////////////////////////////////////////////

ServerNetworkComm::ServerNetworkComm()
{
  regist_signal( "new_client_connected" )
      .description("Event raised whan a new client gets connected and registered.");

//  m_mutex = new QMutex();
}

////////////////////////////////////////////////////////////////////////////

ServerNetworkComm::~ServerNetworkComm()
{
//  QHash<QTcpSocket*, std::string>::iterator it = m_clients.begin();

//  while(it != m_clients.end())
//  {
//    delete it.key();
//    it++;
//  }

//  m_clients.clear();

//  m_server->close();
//  delete m_server;

}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::run()
{
  // those object must be instancied here so they belong to this thread
  m_io_service = new asio::io_service();
  m_acceptor = new tcp::acceptor( *m_io_service, tcp::endpoint( tcp::v4(), m_port ) );

  CFinfo << "Listening on port " << m_acceptor->local_endpoint().port() << CFendl;

  init_accept();

  // blocks until all asynchronous operations are done ("Boost.Asio event loop")
  m_io_service->run();
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::close()
{
  if( is_not_null(m_io_service) )
  {
    m_io_service->stop();
  }
}

////////////////////////////////////////////////////////////////////////////

bool ServerNetworkComm::open_port( unsigned short port )
{
  bool success = false;

  if( is_null(m_acceptor) )
  {
    m_port = port;

    run();

    success = true;
  }

  return success;
}


////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::init_accept()
{
  TCPConnection::Ptr conn = TCPConnection::create( *m_io_service );

  m_acceptor->async_accept( conn->socket(),
                            boost::bind( &ServerNetworkComm::callback_accept,
                                         this,
                                         conn,
                                         asio::placeholders::error
                                       )
                           );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::init_send( TCPConnection::Ptr client, SignalFrame & frame )
{
  if( is_not_null(client) )
  {
    client->send( frame,
                  boost::bind( &ServerNetworkComm::callback_send,
                               this,
                               client,
                               asio::placeholders::error
                               )
                  );
  }
  else // if the connection is null, broadcast the frame
  {
    std::map<TCPConnection::Ptr, ClientInfo>::iterator it = m_clients.begin();

    for( ; it != m_clients.end() ; ++it )
      init_send( it->first, frame );
  }
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::init_read( ClientInfo & client )
{
  client.connection->read( client.buffer,
                           boost::bind( &ServerNetworkComm::callback_read,
                                        this,
                                        client.connection,
                                        asio::placeholders::error
                                      )
                         );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::callback_accept( TCPConnection::Ptr conn,
                                         const boost::system::error_code & error )
{
  if( !error )
  {
    // create client info
    ClientInfo& info = m_clients[conn];

    info.connection = conn;
    info.buffer = SignalFrame( "message", "cpath:/", "cpath:/" );
    info.error_handler = boost::shared_ptr<ErrorHandler>(new ErrorHandler());

    info.connection->set_error_handler( info.error_handler );

    CFinfo << "New client connected from " << conn->socket().remote_endpoint().address()
           << CFendl;

    init_read( info ); // init read operation for the new client
    init_accept();     // wait for other clients
  }
  else
    CFinfo << "Failed to accept a client connection: " << error.message() << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::callback_send( TCPConnection::Ptr conn,
                                       const boost::system::error_code & error )
{
  if ( error )
    CFerror << "Data could not be sent to [" << conn->socket().remote_endpoint()
            << "]: " << error.message() << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::callback_read( TCPConnection::Ptr conn,
                                       const boost::system::error_code & error )
{
  if( !error )
  {
    std::string error_msg;
    ClientInfo& info = m_clients[conn];
    SignalFrame & buffer = info.buffer;

    std::string target = buffer.node.attribute_value( "target" );
    std::string receiver = buffer.node.attribute_value( "receiver" );
    std::string clientid = buffer.node.attribute_value( "clientid" );
    std::string frameid = buffer.node.attribute_value( "frameid" );

    // check if the client is attempting to register
    if( target == "client_registration" )
    {
      if( !info.uuid.empty() )
        error_msg = "This client has already been registered.";
      else
      {
        info.uuid = clientid;

        // Build and send the reply
        SignalFrame reply = buffer.create_reply();
        SignalOptions & roptions = reply.options();

        roptions.add("accepted", true);
        roptions.flush();

        this->init_send(conn, buffer );

        // tell listeners a new client has arrived
        SignalFrame frame("new_client_connected", "cpath:/", "cpath:/");
        frame.options().add( "clientid", clientid );
        call_signal( "new_client_connected", frame );
      }
    }
    else
    {
      if( info.uuid.empty() )
        error_msg = "The signal came from an unregistered client.";
      else if( info.uuid != clientid )
        error_msg = "The client id '" + info.uuid + "' (used for registration) "
            + "and '" + clientid + "' (used for identification) do not match.";
      else
        ServerRoot::instance().process_signal(target, receiver, clientid, frameid, buffer);
    }

    if( !error_msg.empty() )
      this->send_frame_rejected( conn, frameid, SERVER_CORE_PATH, error_msg );

    init_read( info );
  }
  else if( error != boost::asio::error::eof )
    CFerror << "Could not read from [" << conn->socket().remote_endpoint()
            << "]: " << error.message() << CFendl;

  if( error )
  {
    std::string uuid = m_clients[conn].uuid;
    conn->disconnect();
    m_clients.erase( conn );

    if( error == boost::asio::error::eof )
      CFinfo << "Cliemt [" << uuid << "] has disconnected. ("
             << m_clients.size() << " left)." << CFendl;
  }

}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_frame_to_client( SignalFrame & signal,
                                              const string & clientid )
{
  init_send( get_connection(clientid), signal );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_frame_rejected_to_client ( const string clientid,
                                                         const string & frameid,
                                                         const URI & sender,
                                                         const string &reason )
{
  send_frame_rejected( get_connection(clientid), frameid, sender, reason );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_message_to_client( const string & message,
                                                LogMessage::Type type,
                                                const string & clientid )
{
  send_message( get_connection(clientid), message, type );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_frame_rejected( TCPConnection::Ptr client,
                                             const string & frameid,
                                             const URI & sender,
                                             const std::string & reason )
{
  SignalFrame frame("frame_rejected", sender, CLIENT_ROOT_PATH);
  SignalOptions & options = frame.options();

  options.add( "frameid", frameid );
  options.add( "reason", reason );

  init_send( client, frame );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_message( TCPConnection::Ptr client,
                                      const string & message,
                                      LogMessage::Type type )
{
  SignalFrame frame("message", SERVER_CORE_PATH, CLIENT_LOG_PATH);
  SignalOptions & options = frame.options();

  if(type == LogMessage::INVALID)
    type = LogMessage::INFO;

  options.add("type", LogMessage::Convert::instance().to_str(type));
  options.add("text", message);

  init_send( client, frame );
}

////////////////////////////////////////////////////////////////////////////

TCPConnection::Ptr ServerNetworkComm::get_connection( const string & uuid ) const
{
  TCPConnection::Ptr connection;

  if( !uuid.empty() )
  {
    std::map<TCPConnection::Ptr, ClientInfo>::const_iterator it = m_clients.begin();

    for( ; it != m_clients.end() && is_null(connection) ; ++it )
    {
      if( it->second.uuid == uuid )
        connection = it->first;
    }

    if( is_null(connection) )
      throw UnknownClientId(FromHere(), "Unknown client id: " + uuid);
  }

  return connection;
}

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3
