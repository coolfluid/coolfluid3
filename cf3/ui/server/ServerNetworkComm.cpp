// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <iostream>

#include <QtNetwork>
#include <QtCore>

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

  m_mutex = new QMutex();
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

bool ServerNetworkComm::open_port( unsigned short port )
{
  bool success = false;

  if( is_null(m_acceptor) )
  {
    m_port = port;

    start();

//    m_server = new QTcpServer(this);

//    if(!m_server->listen(QHostAddress::Any, port))
//    {
//      QString message = QString("Cannot listen %1 on port %2 : %3")
//                        .arg("")
//                        .arg(port)
//                        .arg(m_server->errorString());
//      throw NetworkError(FromHere(), message.toStdString());
//    }

//    connect(m_server, SIGNAL(newConnection()), this, SLOT(newClient()));
//    m_server->setMaxPendingConnections(1);

    success = true;
  }

  return success;
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send(TCPConnection::Ptr client, const XmlDoc & signal)
{
//  QMutexLocker locker(m_mutex);
//  QByteArray block;
//  QDataStream out(&block, QIODevice::WriteOnly);
//  int count = 0; // total bytes sent

//  std::string signal_str;

//  XML::to_string(signal, signal_str);

//  out.setVersion(QDataStream::Qt_4_6);

//  out.writeBytes(signal_str.c_str(), signal_str.length() + 1);

//  if(client == nullptr)
//  {
//    QHash<TCPConnection::Ptr, std::string>::iterator it = m_clients.begin();
//    while(it != m_clients.end())
//    {
//      client = it.key();
//      count += client->write(block);
//      m_bytesSent += count;
//      it++;
//    }
//  }
//  else
//  {
//    count = client->write(block);
//    m_bytesSent += count;
//  }

//  return count;
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
                                         const system::error_code & error )
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
                                       const system::error_code & error )
{
  if ( error )
    CFerror << "Data could not be sent to [" << conn->socket().remote_endpoint()
            << "]: " << error.message() << CFendl;
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::callback_read( TCPConnection::Ptr conn,
                                       const system::error_code & error )
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

        roptions.add_option("accepted", true);
        roptions.flush();

        this->init_send(conn, buffer );

        // tell listeners a new client has arrived
        SignalFrame frame("new_client_connected", "cpath:/", "cpath:/");
        frame.options().add_option( "clientid", clientid );
        call_signal( "new_client_connected", frame );
      }
    }
    else
    {
      /*if( info.uuid.empty() )
        error_msg = "The signal came from an unregistered client.";
      else*/ if( info.uuid != clientid )
        error_msg = "The client id '" + info.uuid + "' (used for registration) "
            + "and '" + clientid + "' (used for identification) do not match.";
      else
        ServerRoot::instance().process_signal(target, receiver, clientid, frameid, buffer);
    }

    if( !error_msg.empty() )
      this->send_frame_rejected( conn, frameid, SERVER_CORE_PATH, error_msg );

    init_read( info );
  }
  else
    CFerror << "Could not read from [" << conn->socket().remote_endpoint()
            << "]: " << error.message() << CFendl;

}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_frame_to_client( SignalFrame & signal,
                                              const string & clientid )
{
  init_send( get_connection(clientid), signal );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_frame_rejected_to_client( const string clientid,
                                                       const string & frameid,
                                                       const URI & sender,
                                                       const QString & reason )
{
  send_frame_rejected( get_connection(clientid), frameid, sender, reason.toStdString() );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_message_to_client( const QString & message,
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

  options.add_option( "frameid", frameid );
  options.add_option( "reason", reason );

//  options.flush();

  init_send( client, frame );
}

////////////////////////////////////////////////////////////////////////////

void ServerNetworkComm::send_message( TCPConnection::Ptr client,
                                      const QString & message,
                                      LogMessage::Type type )
{
  SignalFrame frame("message", SERVER_CORE_PATH, CLIENT_LOG_PATH);
  SignalOptions & options = frame.options();

  if(type == LogMessage::INVALID)
    type = LogMessage::INFO;

  options.add_option("type", LogMessage::Convert::instance().to_str(type));
  options.add_option("text", message.toStdString());

//  options.flush();
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


/****************************************************************************

SLOTS

*****************************************************************************/

//void ServerNetworkComm::newClient()
//{
//  TCPConnection::Ptr socket;

//  socket = m_server->nextPendingConnection();

//  // connect useful signals to slots
//  connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
//  connect(socket, SIGNAL(readyRead()), this, SLOT(newData()));

//  std::cout << "A new client is connected" << std::endl;
//}

////////////////////////////////////////////////////////////////////////////

//void ServerNetworkComm::newData()
//{
//  // which client has sent data ?
//  TCPConnection::Ptr socket = qobject_cast<TCPConnection::Ptr>(sender());
//  std::string target;
//  std::string receiver;
//  std::string clientId;
//  std::string frameId;

//  QString errorMsg;

//  try
//  {
//    char * frame;
//    QDataStream in(socket);

//    in.setVersion(QDataStream::Qt_4_6); // set stream version

//    // if the client sends two messages very close in time, it is possible that
//    // the server never gets the second one.
//    // So, it is useful to explicitly read the socket until the end is reached.
//    while(!socket->atEnd())
//    {
//      in.readBytes(frame, m_blockSize);

//      m_bytesRecieved += m_blockSize + (int)sizeof(quint32);

//      boost::shared_ptr< XmlDoc > xmldoc = XML::parse_cstring( frame, m_blockSize - 1 );

////      std::cout << frame << std::endl;

//      // free the buffer
//      delete[] frame;
//      frame = nullptr;

//      XmlNode nodedoc = Protocol::goto_doc_node(*xmldoc.get());
//      SignalFrame * sig_frame = new SignalFrame( nodedoc.content->first_node() );

//      sig_frame->xml_doc = xmldoc;

//      target = this->get_attr(sig_frame->node, "target", errorMsg);
//      receiver = this->get_attr(sig_frame->node, "receiver", errorMsg);
//      clientId = this->get_attr(sig_frame->node, "clientid", errorMsg);
//      frameId = this->get_attr(sig_frame->node, "frameid", errorMsg);

//      if( errorMsg.isEmpty() )
//      {
//        if(target == "client_registration")
//        {
//          if(!m_clients[socket].empty())
//            errorMsg = "This client has already been registered.";
//          else
//          {
//            m_clients[socket] = clientId;

//            // Build the reply
//            SignalFrame reply = sig_frame->create_reply();
//            SignalOptions roptions( reply );

//            roptions.add_option("accepted", true);

//            roptions.flush();

//            this->send(socket, *xmldoc.get());

//            emit newClientConnected(clientId);
//          }
//        }
//        else
//        {
//          if( m_clients[socket].empty() )
//            errorMsg = "The signal came from an unregistered client.";
//          else if( m_clients[socket] != clientId )
//            errorMsg = QString("The client id '%1' (used for registration) "
//                               "and '%2' (used for identification) do not "
//                               "match.").arg(m_clients[socket].c_str()).arg(clientId.c_str());
//          else
//            ServerRoot::instance().process_signal(target, receiver, clientId, frameId, *sig_frame);
//        }
//      }

//      m_blockSize = 0;
//    }
//  }
//  catch(Exception & e)
//  {
//    this->send_message(socket, e.what(), LogMessage::EXCEPTION);
//  }
//  catch(std::exception & stde)
//  {
//    this->send_message(socket, stde.what(), LogMessage::EXCEPTION);
//  }
//  catch(...)
//  {
//    errorMsg = QString("An unknown exception has been caught.");
//  }

//  if(!errorMsg.isEmpty())
//    this->send_frame_rejected(socket, frameId, SERVER_CORE_PATH, errorMsg);

//}

////////////////////////////////////////////////////////////////////////////

//void ServerNetworkComm::clientDisconnected()
//{
//  // which client has been disconnected ?
//  TCPConnection::Ptr socket = qobject_cast<TCPConnection::Ptr>(sender());

//  if(socket != nullptr)
//  {
//    m_clients.remove(socket);

//    std::cout << "A client has gone (" << m_clients.size() << " left)\n";
//  }
//}

////////////////////////////////////////////////////////////////////////////

//void ServerNetworkComm::message(const QString & message)
//{
//  this->send_message( TCPConnection::Ptr(), message, LogMessage::INFO);
//}

////////////////////////////////////////////////////////////////////////////

//void ServerNetworkComm::error(const QString & message)
//{
//  this->send_message(TCPConnection::Ptr(), message, LogMessage::ERROR);
//}

////////////////////////////////////////////////////////////////////////////

} // Server
} // ui
} // cf3
