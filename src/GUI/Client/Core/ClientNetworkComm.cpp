// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QByteArray>
#include <QDataStream>
#include <QHostInfo>
#include <QTcpSocket>

#include <string>

#include "Common/CF.hpp"
#include "Common/Core.hpp"
#include "Common/URI.hpp"
#include "Common/BasicExceptions.hpp"

#include "GUI/Client/Core/NLog.hpp"
#include "GUI/Client/Core/ClientRoot.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/NetworkException.hpp"


#include "GUI/Client/Core/ClientNetworkComm.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::Network;

ClientNetworkComm::ClientNetworkComm()
{
  m_socket = new QTcpSocket(this);

  // connect useful signals to slots
  connect(m_socket, SIGNAL(readyRead()), this, SLOT(newData()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(socketError(QAbstractSocket::SocketError)));

  connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));

  m_blockSize = 0;
  m_requestDisc = false;
  m_skipRefused = false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ClientNetworkComm::~ClientNetworkComm()
{
  delete m_socket;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::connectToServer(const QString & hostAddress, quint16 port,
                                        bool skipRefused)
{
  m_skipRefused = skipRefused;
  m_socket->connectToHost(hostAddress, port);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::disconnectFromServer(bool shutServer)
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::isConnected() const
{
  return m_socket->state() == QAbstractSocket::ConnectedState;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ClientNetworkComm::send(const QString & frame) const
{
  int charsWritten;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);

  out.setVersion(QDataStream::Qt_4_6); // QDataStream version
  out << (quint32)0;    // reserve 32 bits for the frame data size
  out << frame;
  out.device()->seek(0);  // go back to the beginning of the frame
  out << (quint32)(block.size() - sizeof(quint32)); // write the frame data size

  charsWritten = m_socket->write(block);
  m_socket->flush();

  return charsWritten;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::send(Signal::arg_t & signal)
{
  cf_assert ( signal.node.is_valid() );
  cf_assert ( is_not_null(signal.xml_doc.get()) );
  bool success = false;
  std::string str;

  if(this->checkConnected())
  {
    signal.node.set_attribute( "clientid", ClientRoot::instance().getUUID() );

    signal.xml_doc->to_string(str);

    success = this->send(str.c_str()) > 0;
  }

  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::saveNetworkInfo () const
{
  NetworkInfo & info = Core::instance().network_info();

  info.set_hostname( QHostInfo::localHostName().toStdString() );
  info.set_port( m_socket->peerPort() );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::checkConnected()
{
  bool connected = isConnected();

  if(!connected)
    NLog::globalLog()->addError("Not connected to the server.");

  return connected;
}

/****************************************************************************

                                    SLOTS

 ****************************************************************************/

void ClientNetworkComm::newData()
{
  QString frame;
  QDataStream in(m_socket);
  in.setVersion(QDataStream::Qt_4_6); // QDataStream version

  // if the server sends two messages very close in time, it is possible that
  // the client never gets the second one.
  // So, it is useful to explicitly read the socket until the end is reached.
  while(!m_socket->atEnd())
  {
    // if the data size is not known
    if (m_blockSize == 0)
    {
      // if there are at least 4 bytes to read...
      if (m_socket->bytesAvailable() < (int)sizeof(quint32))
        return;

      // ...we read them
      in >> m_blockSize;
    }

    if (m_socket->bytesAvailable() < m_blockSize)
      return;

    in >> frame;

    //qDebug() << frame;

    try
    {
      ClientRoot::instance().processSignalString(frame);
    }
    catch(Exception & e)
    {
      NLog::globalLog()->addException(e.what());
    }

    m_blockSize = 0;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::disconnected()
{
  if(!m_requestDisc)
  {
    NLog::globalLog()->addError("The connection has been closed.");
  }
  emit disconnectedFromServer();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::socketError(QAbstractSocket::SocketError err)
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
      if(!m_skipRefused)
        log->addError("Connection refused. Please check if the server is running.");
      else
        m_skipRefused = false;
      break;

    default:
      log->addError(QString("The following error occurred: ") + m_socket->errorString());
  }
}
