// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QMutexLocker>
#include <QTcpSocket>

#include "Common/Log.hpp"
#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/FileOperations.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/NLog.hpp"
#include "UI/Core/NTree.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NetworkThread.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

NetworkThread::NetworkThread(QObject *parent) :
    QThread(parent),
    m_socket(new QTcpSocket()),
    m_blockSize(0),
    m_port(0),
    m_requestDisc(false)
{
  qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread::~NetworkThread()
{
  m_requestDisc = false;
  m_socket->disconnectFromHost();

  if(isRunning())
  {
    exit(0);
    wait();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::connectToHost(const QString &hostAddress, quint16 port)
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

bool NetworkThread::isConnected() const
{
  if( is_null(m_socket) )
    return false;

  return m_socket->state() == QAbstractSocket::ConnectedState;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnectFromServer(bool shutServer)
{
//  QMutexLocker locker(&m_mutex);


  if(isConnected())
  {
    if(shutServer)
    {
      SignalFrame frame("shutdown", CLIENT_ROOT_PATH, SERVER_CORE_PATH);

      this->send(frame);
    }

    m_requestDisc = true;

    // close the socket
//    m_socket->abort();
//    m_socket->close();
  }
}

////////////////////////////////////////////////////////////////////////////////

int NetworkThread::send(Common::SignalArgs& signal)
{
  if(!isConnected())
    throw IllegalCall(FromHere(), "There is no active connection.");

  int charsWritten;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  std::string str;

  out.setVersion(QDataStream::Qt_4_6); // set stream version

  signal.node.set_attribute( "clientid", ThreadManager::instance().tree().getUUID() );

  to_string(*signal.xml_doc, str);

  out.writeBytes(str.c_str(), str.length() + 1);

  charsWritten = m_socket->write(block);

#ifdef Q_WS_MAC
  m_socket->flush();
#endif

  return charsWritten;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::newData()
{
  char * frame = nullptr;
  QDataStream in(m_socket);

  in.setVersion(QDataStream::Qt_4_6); // set stream version

  // if the server sends two messages very close in time, it is possible that
  // the client never gets the second one.
  // So, it is useful to explicitly read the socket until the end is reached.
  while(!m_socket->atEnd())
  {
    in.readBytes(frame, m_blockSize);

    if(NTree::globalTree()->isDebugModeEnabled())
      CFinfo << frame << CFendl;

    // parse the frame and call the boost signal
    try
    {
      if( m_blockSize > 0 )
      {
        XmlDoc::Ptr doc = XML::parse_cstring(frame, m_blockSize - 1);
        newSignal(doc);
      }
    }
    catch(XmlError & e)
    {
      NLog::globalLog()->addException(e.what());
    }

    // free the buffer
    delete[] frame;
    frame = nullptr;

    m_blockSize = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::run()
{
  delete m_socket;
  m_socket = new QTcpSocket();

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(newData()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(socketError(QAbstractSocket::SocketError)));

  connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));

  m_socket->connectToHost(m_hostname, m_port);

  if (!m_socket->waitForConnected())
  {
    socketError(m_socket->error());
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
  if(!m_requestDisc)
    NLog::globalLog()->addError("The connection has been closed.");
  else
    NLog::globalLog()->addMessage("Disconnected from the server.");

  if(isRunning())
    exit( m_requestDisc ? 0 : 1 );

  emit disconnectedFromServer(m_requestDisc);
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::socketError(QAbstractSocket::SocketError err)
{
  if(m_requestDisc)
    return;

  if( isConnected() )
  {
    m_requestDisc = true;
    m_socket->disconnectFromHost();
  }

  NLog::globalLog()->addError(QString("Network error: %1").arg(m_socket->errorString()));
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF
