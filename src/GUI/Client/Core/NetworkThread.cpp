// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QTcpSocket>

#include "Common/Log.hpp"

#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/NTree.hpp"

#include "GUI/Client/Core/NetworkThread.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

NetworkThread::NetworkThread(QObject *parent) :
    QThread(parent),
    m_blockSize(0),
    m_requestDisc(false)
{
  m_socket = new QTcpSocket(this);

  connect(m_socket, SIGNAL(readyRead()), this, SLOT(newData()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(socketError(QAbstractSocket::SocketError)));

  connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));
}

////////////////////////////////////////////////////////////////////////////////

NetworkThread::~NetworkThread()
{

}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::connectToHost(const QHostAddress &hostAddress, quint16 port)
{
  if(!isRunning())
  {
    m_socket->connectToHost(hostAddress, port);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool NetworkThread::isConnected() const
{
  return m_socket->state() == QAbstractSocket::ConnectedState;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnect(bool shutServer)
{
  if(shutServer)
  {
    SignalFrame frame("shutdown", CLIENT_CORE_PATH, SERVER_CORE_PATH);

    this->send(frame);
  }

  m_requestDisc = true;

  // close the socket
  m_socket->abort();
  m_socket->close();
}

////////////////////////////////////////////////////////////////////////////////

int NetworkThread::send(Common::Signal::arg_t& signal)
{
  if(!isConnected())
    throw IllegalCall(FromHere(), "There is no active connection.");

  int charsWritten;

  QByteArray block;
  QTextStream out(&block, QIODevice::WriteOnly);
  std::string str;

  signal.node.set_attribute( "clientid", ""/*ClientRoot::instance().getUUID()*/ );

  signal.xml_doc->to_string(str);

  out << (quint32)0;    // reserve 32 bits for the frame data size
  out << str.c_str();
  out.device()->seek(0);  // go back to the beginning of the frame
  out << (quint32)(block.size() - sizeof(quint32)); // write the frame data size

  charsWritten = m_socket->write(block);
  m_socket->flush();

  return charsWritten;
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::newData()
{
  char * frame = nullptr;
  QTextStream in(m_socket);

  // if the server sends two messages very close in time, it is possible that
  // the client never gets the second one.
  // So, it is useful to explicitly read the socket until the end is reached.
  while(!m_socket->atEnd())
  {
    // if the data size is not known
    if (m_blockSize == 0)
    {
      // if there are at least 4 bytes to read...
      if (m_socket->bytesAvailable() < (quint64)sizeof(quint32))
        return;

      // ...we read them
      in >> m_blockSize;
    }

    if (m_socket->bytesAvailable() < m_blockSize)
      return;

    frame = new char[m_blockSize];
    in >> frame;

    if(NTree::globalTree()->isDebugModeEnabled())
      CFinfo << frame << CFendl;

    // parse the frame and call the boost signal
    try
    {
      XmlDoc::Ptr doc = XmlDoc::parse_string(frame);
      newSignal(doc);
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
  // execute the event loop
  exec();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::disconnected()
{
  if(!m_requestDisc)
  {
    NLog::globalLog()->addError("The connection has been closed.");
  }

  if(isRunning())
    exit( m_requestDisc ? 0 : 1 );

  emit disconnectedFromServer();
}

////////////////////////////////////////////////////////////////////////////////

void NetworkThread::socketError(QAbstractSocket::SocketError err)
{
  NLog::Ptr log = NLog::globalLog();

  if(m_requestDisc)
    return;

  if( isConnected() )
  {
    m_requestDisc = true;
    m_socket->disconnectFromHost();
  }

  switch (err)
  {
    case QAbstractSocket::RemoteHostClosedError:
      log->addError("Remote connection closed");
      break;

    case QAbstractSocket::HostNotFoundError:
      log->addError("Host was not found");
      break;

    case QAbstractSocket::ConnectionRefusedError:
      log->addError("Connection refused. Please check if the server is running.");
      break;

    default:
      log->addError(QString("The following error occurred: ") + m_socket->errorString());
  }
}


} // ClientCore
} // GUI
} // CF
