// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutexLocker>
#include <QTcpSocket>

#include "common/Log.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/FileOperations.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NetworkThread.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

NetworkThread::NetworkThread(QObject *parent) :
    QThread(parent),
    m_socket(new QTcpSocket()),
    m_block_size(0),
    m_port(0),
    m_request_disc(false)
{
  qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread::~NetworkThread()
{
  m_request_disc = false;
  m_socket->disconnectFromHost();

  if(isRunning())
  {
    exit(0);
    wait();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::connect_to_host(const QString &hostAddress, quint16 port)
{
  if(!isRunning())
  {
    m_hostname = hostAddress;
    m_port = port;
    start();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::is_connected() const
{
  if( is_null(m_socket) )
    return false;

  return m_socket->state() == QAbstractSocket::ConnectedState;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnect_from_server(bool shutServer)
{
//  QMutexLocker locker(&m_mutex);


  if(is_connected())
  {
    if(shutServer)
    {
      SignalFrame frame("shutdown", CLIENT_ROOT_PATH, SERVER_CORE_PATH);

      this->send(frame);
    }

    m_request_disc = true;

    // close the socket
//    m_socket->abort();
//    m_socket->close();
  }
}

////////////////////////////////////////////////////////////////////////////////

int NetworkThread::send(common::SignalArgs& signal)
{
  if(!is_connected())
    throw IllegalCall(FromHere(), "There is no active connection.");

  int charsWritten;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  std::string str;

  out.setVersion(QDataStream::Qt_4_6); // set stream version

  signal.node.set_attribute( "clientid", ThreadManager::instance().tree().get_uuid() );

  to_string(*signal.xml_doc, str);

  out.writeBytes(str.c_str(), str.length() + 1);

  charsWritten = m_socket->write(block);

#ifdef Q_WS_MAC
  m_socket->flush();
#endif

  return charsWritten;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::new_data()
{
  char * frame = nullptr;
  QDataStream in(m_socket);

  in.setVersion(QDataStream::Qt_4_6); // set stream version

  // if the server sends two messages very close in time, it is possible that
  // the client never gets the second one.
  // So, it is useful to explicitly read the socket until the end is reached.
  while(!m_socket->atEnd())
  {
    in.readBytes(frame, m_block_size);
    
    std::string frame_str;
    frame_str.reserve(m_block_size);
    
    char* frame_part = frame;
    quint32 offset = frame_part - frame;

    while(offset < m_block_size)
    {
      frame_str += frame_part;
      frame_part += strlen(frame_part)+1;
      offset = frame_part - frame;
    }
    cf3_assert(offset == m_block_size);

    if(NTree::global()->is_debug_mode_enabled())
      CFinfo << frame_str << CFendl;

    // parse the frame and call the boost signal
    try
    {
      if( m_block_size > 0 )
      {
        XmlDoc::Ptr doc = XML::parse_string(frame_str);
        newSignal(doc);
      }
    }
    catch(XmlError & e)
    {
      NLog::global()->add_exception(e.what());
    }

    // free the buffer
    delete[] frame;
    frame = nullptr;

    m_block_size = 0;
  }
}
 
////////////////////////////////////////////////////////////////////////////////

void NetworkThread::run()
{
  delete m_socket;
  m_socket = new QTcpSocket();

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(new_data()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(socket_error(QAbstractSocket::SocketError)));

  connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));

  m_socket->connectToHost(m_hostname, m_port);

  if (!m_socket->waitForConnected())
  {
    socket_error(m_socket->error());
    return;
  }

  // execute the event loop
  exec();

  delete m_socket;
  m_socket = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnected()
{
  if(!m_request_disc)
    NLog::global()->add_error("The connection has been closed.");
  else
    NLog::global()->add_message("Disconnected from the server.");

  if(isRunning())
    exit( m_request_disc ? 0 : 1 );

  emit disconnected_from_server(m_request_disc);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::socket_error(QAbstractSocket::SocketError err)
{
  if(m_request_disc)
    return;

  if( is_connected() )
  {
    m_request_disc = true;
    m_socket->disconnectFromHost();
  }

  NLog::global()->add_error(QString("Network error: %1").arg(m_socket->errorString()));
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
