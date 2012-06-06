// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ui network Connection class"

#include <iostream>

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/TypeInfo.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/FileOperations.hpp"

#include "ui/network/TCPConnection.hpp"
#include "ui/network/ErrorHandler.hpp"

#define NETWORK_PORT 62784
#define NETWORK_HOST "127.0.0.1"

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common::XML;
using namespace cf3::ui::network;


//////////////////////////////////////////////////////////////////////////////

struct LastCallbackInfo
{
public:

  enum Action { INVALID, READ, SEND, ACCEPT, CONNECT };

  Action action;
  TCPConnection::ConstPtr connection;
  boost::system::error_code error_raised;

}; // LastCallbackInfo

//////////////////////////////////////////////////////////////////////////////

class MyErrorHandler : public ErrorHandler
{
public:

  virtual void error(const std::string &message)
  {
    messages.push_back( message );
  }

  std::vector< std::string > messages;

}; // MyErrorHandler

//////////////////////////////////////////////////////////////////////////////

SignalFrame generate_message_frame( const std::string & message )
{
  SignalFrame frame( "message", "cpath:/", "cpath:/" );

  frame.set_option( "text", cf3::common::class_name<std::string>(), message );

  return frame;
}

//////////////////////////////////////////////////////////////////////////////

std::string get_message( const SignalFrame & frame )
{
  return frame.options().value<std::string>( "text" );;
}

//////////////////////////////////////////////////////////////////////////////

class Client
{

public:

  Client( asio::io_service& ios )
    : io_service( ios ),
      endpoint( address::from_string(NETWORK_HOST), NETWORK_PORT ),
      error_handler(new ErrorHandler())
  {
    init_connect( endpoint );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_connect( tcp::endpoint& endpoint )
  {
    connection = TCPConnection::create( io_service );
    tcp::socket& socket = connection->socket();

    connection->set_error_handler(error_handler);

    socket.async_connect( endpoint,
                          boost::bind( &Client::callback_connect,
                                       this,
                                       asio::placeholders::error )
                          );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_read()
  {
    connection->read( args,
                      boost::bind( &Client::callback_read,
                                   this,
                                   asio::placeholders::error )
                      );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_send( SignalFrame & frame )
  {
    connection->send( frame,
                      boost::bind( &Client::callback_send,
                                   this,
                                   asio::placeholders::error
                                   )
                      );
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_send( const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::SEND;
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_connect( const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::CONNECT;

    // on error, socket is open but not connected
    // hence, we need to explicitely close or a runtime error will be raised
    // when trying to shut the connection down in Connection destructor.
    if(error)
      connection->socket().close();
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_read( const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::READ;
  }

public: // data (breaks encapsulation to uTests purpose)

  tcp::endpoint endpoint;
  TCPConnection::Ptr connection;
  SignalFrame args;
  asio::io_service & io_service;
  boost::shared_ptr<ErrorHandler> error_handler;
  LastCallbackInfo last_callback_info;

}; // Client

//////////////////////////////////////////////////////////////////////////////

class Server
{
public: // nested structs

  struct ClientInfo
  {

  public:
    TCPConnection::Ptr connection;

    SignalFrame buffer;

    boost::shared_ptr<ErrorHandler> error_handler;

    cf3::Uint id;

  }; // ClientInfo


public:

  Server( asio::io_service& io_service ) :
      m_acceptor( io_service, tcp::endpoint( tcp::v4(), NETWORK_PORT ) )
  {
    init_accept();
  }

  void init_accept()
  {
    TCPConnection::Ptr conn = TCPConnection::create( m_acceptor.get_io_service() );

    m_acceptor.async_accept( conn->socket(),
                             boost::bind( &Server::callback_accept,
                                          this,
                                          conn,// 1st param for the callback fct
                                          asio::placeholders::error ) ); // 2nd param
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_read( TCPConnection::Ptr conn, SignalFrame & buffer )
  {
    conn->read( buffer,
                boost::bind( &Server::callback_read,
                             this,
                             conn, // 1st param for the callback fct
                             asio::placeholders::error ) // 2nd param
              );
  }

  /////////////////////////////////////////////////////////////////////////////

  void init_send( TCPConnection::Ptr conn, SignalFrame & buffer )
  {
    conn->send( buffer,
                boost::bind( &Server::callback_send,
                             this,
                             conn, // 1st param for the callback fct
                             asio::placeholders::error // 2nd param
                           )
              );
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_accept( TCPConnection::Ptr conn, const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::ACCEPT;
    last_callback_info.connection = conn;

    ClientInfo& info = m_clients[conn];

    info.connection = conn;
    info.buffer = SignalFrame( "message", "cpath:/", "cpath:/" );
    info.error_handler = boost::shared_ptr<ErrorHandler>(new ErrorHandler());
    info.id = m_clients.size() - 1; // start at 0
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_send( TCPConnection::Ptr conn, const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::SEND;
    last_callback_info.connection = conn;

    // if error is "End of file", client is disconnected => remove the socket
    if( error == boost::asio::error::eof )
      m_clients.erase(conn);
  }

  /////////////////////////////////////////////////////////////////////////////

  void callback_read( TCPConnection::Ptr conn, const boost::system::error_code & error )
  {
    last_callback_info.error_raised = error;
    last_callback_info.action = LastCallbackInfo::READ;
    last_callback_info.connection = conn;

    // if error is "End of file", client is disconnected => remove the socket
    if( error == boost::asio::error::eof )
      m_clients.erase(conn);
  }

public: // data (breaks encapsulation to uTests purpose)

  std::map<TCPConnection::ConstPtr, ClientInfo> m_clients;
  tcp::acceptor m_acceptor;

  LastCallbackInfo last_callback_info;

}; // Server

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( uiNetworkConnectionSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( connect_failure )
{
  asio::io_service ios;
  Client client( ios );
  LastCallbackInfo & info = client.last_callback_info;

  // no server, client should fail to connect (connection refused)

  ios.run(); // wait for asynchronous operation (connect) to finish

  BOOST_CHECK_EQUAL ( info.action, LastCallbackInfo::CONNECT );
  BOOST_CHECK_EQUAL ( info.error_raised, asio::error::connection_refused );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( multi_client_read )
{
  asio::io_service ios_server;
  Server server ( ios_server );
  LastCallbackInfo & info_server = server.last_callback_info;
  std::vector<Client*> clients;
  std::vector<asio::io_service*> clients_ios(3); // pointers because io_service is noncopyable
  std::vector<std::string> client_msgs(3);

  client_msgs[0] = "I am client ONE: the first one that has been created!";
  client_msgs[1] = "I am client TWO: the second one that has been created!";
  client_msgs[2] = "I am client THREE: the last one that has been created!";

  clients_ios[0] = new asio::io_service();
  clients_ios[1] = new asio::io_service();
  clients_ios[2] = new asio::io_service();

  //////////////////
  // 1. connect the clients
  //////////////////

  for ( int i = 0 ; i < 3 ; ++i )
  {
    asio::io_service & ios = *clients_ios[i];
    Client * client = new Client( ios );
    LastCallbackInfo & info = client->last_callback_info;

    server.init_accept();

    // wait for the connection to proceed
    ios.run();
    ios_server.run_one(); // wait for at most one operation to finish

    clients.push_back( client );
  }

  //////////////////
  // 2. each client sends a different string to the server
  //////////////////

  TCPConnection::Ptr conn;

  std::map<TCPConnection::ConstPtr, Server::ClientInfo>::iterator it = server.m_clients.begin();

  // initiate a read on all clients
  for( ; it != server.m_clients.end() ; ++it )
    server.init_read( it->second.connection, it->second.buffer );

  // send data from clients
  for( int i = 0 ; i < 3 ; ++i )
  {
    SignalFrame frame = generate_message_frame(client_msgs[i]);
    clients[i]->init_send( frame );
  }

  return;

  // process read data
  for ( int i = 0 ; i < 3 ; ++i )
  {
    int cnt =  ios_server.run_one(); // returns after at most 1 async operation hsa finished

    // check the right callback was called and the operation success state
    BOOST_CHECK_EQUAL ( info_server.action, LastCallbackInfo::READ );
    BOOST_CHECK_EQUAL ( info_server.error_raised, boost::system::errc::success );

    // init some variables
    Server::ClientInfo & client = server.m_clients[ info_server.connection ];
    int client_id = client.id;
    std::string msg;

    to_string( *client.buffer.xml_doc.get(), msg );
    std::cout << msg << std::endl;

    BOOST_REQUIRE_NO_THROW ( msg = get_message( client.buffer ) );

    BOOST_CHECK_EQUAL( msg, client_msgs[client_id] );
  }

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( multi_client_send )
{
  // 1 server, 3 clients connected to it. 3 phases:
  // phase 1: the clients connect to the server
  // phase 2: each sends a string, the server has to receive them and identify the client that spoke
  // phase 3: server sends a string to each client, each of them has to receive the correct message

  asio::io_service ios_server;
  Server server ( ios_server );
  LastCallbackInfo & info_server = server.last_callback_info;
  std::vector<Client*> clients;
  std::vector<asio::io_service*> clients_ios(3); // pointers because io_service is noncopyable
  std::vector<std::string> client_msgs(3);
  std::vector<std::string> server_msgs(3);

  client_msgs[0] = "I am client ONE: the first one that has been created!";
  client_msgs[1] = "I am client TWO: the second one that has been created!";
  client_msgs[2] = "I am client THREE: the last one that has been created!";

  server_msgs[0] = "Hello client ONE, how is it going?";
  server_msgs[1] = "Hello client TWO, is the weather nice at your place?";
  server_msgs[2] = "Hello client THREE, it is quite cold here!";

  clients_ios[0] = new asio::io_service();
  clients_ios[1] = new asio::io_service();
  clients_ios[2] = new asio::io_service();

  //////////////////
  // 1. connect the three clients to the server
  //////////////////

  for ( int i = 0 ; i < 3 ; ++i )
  {
    asio::io_service & ios = *clients_ios[i];
    Client * client = new Client( ios );
    LastCallbackInfo & info = client->last_callback_info;

    server.init_accept();

    // wait for the connection to proceed
    ios.run();
    ios_server.run_one(); // wait for at most one operation to finish

    // check everythings is OK
    BOOST_CHECK_EQUAL ( info.action, LastCallbackInfo::CONNECT );
    BOOST_CHECK_EQUAL ( info.error_raised, boost::system::errc::success );

    BOOST_CHECK_EQUAL ( info_server.action, LastCallbackInfo::ACCEPT );
    BOOST_CHECK_EQUAL ( info_server.error_raised, boost::system::errc::success );

    clients.push_back( client );
  }

  // server should have 3 clients
//  BOOST_REQUIRE_EQUAL ( server.m_clients.size(), size_t(3) );

  //////////////////
  // 2. each client sends a different string to the server
  //////////////////

  //
  // a. send strings
  //
  TCPConnection::Ptr conn;

  std::map<TCPConnection::ConstPtr, Server::ClientInfo>::iterator it = server.m_clients.begin();

  server.last_callback_info = LastCallbackInfo();

  for( ; it != server.m_clients.end() ; ++it )
    server.init_read( it->second.connection, it->second.buffer );

  for( int i = 0 ; i < 3 ; ++i )
  {
    SignalFrame frame = generate_message_frame(client_msgs[i]);
    clients[i]->init_send( frame );
  }

  return;

  for ( int i = 0 ; i < 3 ; ++i )
  {
   int cnt =  ios_server.run_one();

   std::cout << cnt << std::endl;

    BOOST_CHECK_EQUAL ( info_server.action, LastCallbackInfo::READ );
    BOOST_CHECK_EQUAL ( info_server.error_raised, boost::system::errc::success );

//    conn = info_server.connection;

    // must be present as a client
//    BOOST_CHECK ( server.m_clients.find(conn) != server.m_clients.end() );

    Server::ClientInfo & client = server.m_clients[ info_server.connection ];
    int client_id = client.id;
    std::string msg;

    to_string( *client.buffer.xml_doc.get(), msg );
    std::cout << msg << std::endl;

    BOOST_REQUIRE_NO_THROW ( msg = get_message( client.buffer ) );

    BOOST_CHECK_EQUAL( msg, client_msgs[client_id] );
  }


  // 1. server accepts 3 clients and sends a different string to each of them
  // 2. each client sends a string, the srever should be able to tell from which
  // client the come
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( disconnect )
{
  // 1. server closes the connection, client should throw an error (eof)
  // 2. client closes the connection, server should remove the client from the vector
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( bad_header )
{
  // 1. header is not a valid interger value
  // 2. header value is too small
  // 3. header value is to big
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( bad_data )
{
  // data is not valid XML
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
