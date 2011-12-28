// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutexLocker>
#include <QTcpSocket>
#include <QDebug>

#include <boost/regex.hpp>


#include "common/Log.hpp"
#include "common/XML/FileOperations.hpp"

#include "ui/uicommon/ComponentNames.hpp"

#include "ui/network/TCPConnection.hpp"

#include "ui/core/TreeThread.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NTree.hpp"
#include "ui/core/ThreadManager.hpp"

#include "ui/core/NetworkThread.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace boost;
using namespace boost::asio::ip;

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::network;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////////

NetworkThread::NetworkThread(QObject *parent)
  : QThread(parent),
    m_port(0),
    m_request_disc(false),
    m_endpoint(nullptr)
{
  m_io_service = new asio::io_service();
  regist_signal( "new_network_frame" );
  regist_signal( "connected_to_server" );
  regist_signal( "disconnected_from_server" );
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread::~NetworkThread()
{
  m_request_disc = false;
  disconnect_from_server(false);

  if(isRunning())
  {
    exit(0);

    if( is_not_null(m_io_service) )
      m_io_service->stop();

    wait();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::connect_to_host(const std::string &host_address, unsigned short port)
{
  if(!isRunning())
  {
//    m_port = port;
//    m_hostname = host_address.c_str();

//    // check whether the host address contains an IP address or a hostname
//    boost::regex expression( "^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$" );

//    if( !regex_match( host_address, expression ) )
//    {
//      // it's not an IP address so we need to resolve the name
//      tcp::resolver resolver( *m_io_service );
//      tcp::resolver::query query ( host_address, to_str<cf3::Uint>(port) );
//      tcp::resolver::iterator it_begin = resolver.resolve( query );
//      tcp::resolver::iterator it_end;

//      if( it_begin == it_end ) // if no addresses were found
//        throw BadValue( FromHere(), "Could not resolve hostname [" + host_address + "].");

//      m_endpoint = new tcp::endpoint ( *it_begin );
//    }
//    else
//      m_endpoint = new tcp::endpoint( address::from_string(host_address), port );

    m_endpoint = new tcp::endpoint( tcp::v4(), port );

    start();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::is_connected() const
{
  if( is_null(m_connection) )
    return false;

  return m_connection->socket().is_open();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnect_from_server(bool shutServer)
{
//  QMutexLocker locker(&m_mutex);

  if(is_connected())
  {
    if(shutServer)
    {
      SignalFrame frame( "shutdown", CLIENT_ROOT_PATH, SERVER_CORE_PATH );

      this->send( frame );
    }

    m_request_disc = true;

    m_connection->disconnect();
  }
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::send ( common::SignalArgs& signal )
{
  if(!is_connected())
    throw IllegalCall(FromHere(), "There is no active connection.");

//  int charsWritten;

  /////////////////////////////////////////////////////////////////////////
//  QByteArray block;
//  QDataStream out(&block, QIODevice::WriteOnly);
//  std::string str;

//  out.setVersion(QDataStream::Qt_4_6); // set stream version

//  signal.node.set_attribute( "clientid", ThreadManager::instance().tree().get_uuid().string() );

//  to_string(*signal.xml_doc, str);

  m_connection->send( signal,
                      boost::bind( &NetworkThread::callback_send,
                                   this,
                                   asio::placeholders::error
                                 )
                      );

//  out.writeBytes(str.c_str(), str.length() + 1);

//  charsWritten = m_socket->write(block);

//#ifdef Q_WS_MAC
//  m_socket->flush();
//#endif

//  return charsWritten;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::new_data( )
{

//  char * frame = nullptr;
//  QDataStream in(m_socket);

//  in.setVersion(QDataStream::Qt_4_6); // set stream version

//  // if the server sends two messages very close in time, it is possible that
//  // the client never gets the second one.
//  // So, it is useful to explicitly read the socket until the end is reached.
//  while(!m_socket->atEnd())
//  {
//    in.readBytes(frame, m_block_size);

//    if(NTree::global()->is_debug_mode_enabled())
//      CFinfo << frame << CFendl;

//    // parse the frame and call the boost signal
//    try
//    {
//      if( m_block_size > 0 )
//      {
//        boost::shared_ptr< XmlDoc > doc = XML::parse_cstring(frame, m_block_size-1);
//        newSignal(doc);
//      }
//    }
//    catch(XmlError & e)
//    {
//      NLog::global()->add_exception(e.what());
//    }

//    // free the buffer
//    delete[] frame;
//    frame = nullptr;

//    m_block_size = 0;
//  }
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::run()
{
  // [the thread execution starts here]

  m_connection = TCPConnection::create( *m_io_service );
  m_connection->socket().async_connect( *m_endpoint,
                                        boost::bind( &NetworkThread::callback_connect,
                                                     this,
                                                     asio::placeholders::error
                                                   )
                                      );

  // the above call does not return before all asynchronous operation are done.
  // since the client constantly reads on the socket, this function will return
  // if the connection has been lost or disconnect_from_server() was called.
  m_io_service->run();

  // if we arrive here, it means that all asynchronous operations are finished
  // hence, we can disconnect from the server an destroy the endpoint object
  delete_ptr(m_endpoint);

  // [the thread execution ends here]

//  delete m_socket;
//  m_socket = new QTcpSocket();

//  connect(m_socket, SIGNAL(readyRead()), this, SLOT(new_data()));
//  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
//  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
//          SLOT(socket_error(QAbstractSocket::SocketError)));

//  connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));

//  m_socket->connectToHost(m_hostname, m_port);

//  if (!m_socket->waitForConnected())
//  {
//    socket_error(m_socket->error());
//    return;
//  }

//  // execute the event loop
//  exec();

//  delete m_socket;
//  m_socket = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnected()
{
//  if(!m_request_disc)
//    NLog::global()->add_error("The connection has been closed.");
//  else
//    NLog::global()->add_message("Disconnected from the server.");

//  if(isRunning())
//    exit( m_request_disc ? 0 : 1 );

//  emit disconnected_from_server(m_request_disc);
}

////////////////////////////////////////////////////////////////////////////////

//void NetworkThread::socket_error(QAbstractSocket::SocketError err)
//{
//  if(m_request_disc)
//    return;

//  if( is_connected() )
//  {
//    m_request_disc = true;
//    m_socket->disconnectFromHost();
//  }

//  NLog::global()->add_error(QString("Network error: %1").arg(m_socket->errorString()));
//}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::init_read()
{
  m_connection->read( m_buffer,
                      boost::bind( &NetworkThread::callback_read,
                                   this,
                                   boost::asio::placeholders::error
                                 )
                    );
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::callback_connect( const boost::system::error_code & error )
{
  if( !error )
  {
    SignalFrame frame;
    call_signal( "connected_to_server", frame );
    init_read();
  }
  else
  {
    NLog::global()->add_error( QString("Could not connect to server: %1").arg(error.message().c_str()) );
    m_connection->socket().close();
  }
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::callback_read( const boost::system::error_code & error )
{
  if( !error )
    call_signal( "new_network_frame", m_buffer );
  else
    NLog::global()->add_error( QString("Could not read data: %1").arg(error.message().c_str()) );

  /// @todo in case of error, we should analyse the error code before initiating
  /// a new read operation (i.e. if the connection has been lost, we could
  /// have an endless loop issue here)
  init_read();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::callback_send( const boost::system::error_code & error )
{
  if( error )
    NLog::global()->add_error( QString("Could not send data: %1").arg(error.message().c_str()) );
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
