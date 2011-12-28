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
  regist_signal( "network_new_frame" );
  regist_signal( "network_connected" );
  regist_signal( "network_disconnected" );
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
    m_port = port;
    m_hostname = host_address.c_str();

    // check whether the host address contains an IP address or a hostname
    boost::regex expression( "^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$" );

    if( !regex_match( host_address, expression ) )
    {
      // it's not an IP address so we need to resolve the name
      tcp::resolver resolver( *m_io_service );
      tcp::resolver::query query ( tcp::v4(), host_address, to_str<cf3::Uint>(port) );
      tcp::resolver::iterator it_begin = resolver.resolve( query );
      tcp::resolver::iterator it_end;

      if( it_begin == it_end ) // if no addresses were found
        throw BadValue( FromHere(), "Could not resolve hostname [" + host_address + "].");

      m_endpoint = new tcp::endpoint ( it_begin->endpoint() );
    }
    else
      m_endpoint = new tcp::endpoint( address::from_string(host_address), port );

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

  m_connection->send( signal,
                      boost::bind( &NetworkThread::callback_send,
                                   this,
                                   asio::placeholders::error
                                 )
                      );

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
}


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
    call_signal( "network_connected", frame );
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
    call_signal( "network_new_frame", m_buffer );
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
