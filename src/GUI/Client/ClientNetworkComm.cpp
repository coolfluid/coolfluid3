#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <string>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

#include "GUI/Client/NLog.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/HostInfos.hpp"
#include "GUI/Network/NetworkException.hpp"
#include "GUI/Network/SignalInfo.hpp"


#include "GUI/Client/ClientNetworkComm.hpp"

using namespace CF::GUI::Client;
using namespace CF::GUI::Network;
using namespace CF::Common;

ClientNetworkComm::ClientNetworkComm()
{
  m_socket = new QTcpSocket(this);

  // connect useful signals to slots
  connect(m_socket, SIGNAL(readyRead()), this, SLOT(newData()));
  connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
  connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
          SLOT(socketError(QAbstractSocket::SocketError)));

  connect(m_socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));

  m_blockSize = 0;
  m_requestDisc = false;
  m_connectedToServer = false;
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
//  if(shutServer)
//    this->buildAndSend(NETWORK_SHUTDOWN_SERVER);

  m_requestDisc = true;
  m_connectedToServer = false;

  // close the m_socket
  m_socket->abort();
  m_socket->close();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::isConnected() const
{
  return m_connectedToServer;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ClientNetworkComm::send(const QString & frame) const
{
  int charsWritten;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);

  out.setVersion(QDataStream::Qt_4_5); // QDataStream version
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

bool ClientNetworkComm::send(const XmlDoc & signal)
{
  bool success = false;
  std::string str;

  if(this->checkConnected())
  {
    XmlOps::xml_to_string(signal, str);

    this->send(str.c_str());

//    try
//    {
//      QByteArray block;
//      QDataStream out(&block, QIODevice::WriteOnly);

//      XmlOps::xml_to_string(signal, str);

//      out.setVersion(QDataStream::Qt_4_5); // QDataStream version
//      out << (quint32)0;    // reserve 32 bits for the frame data size
//      out << str.c_str();
//      out.device()->seek(0);  // go back to the beginning of the frame
//      out << (quint32)(block.size() - sizeof(quint32)); // write the frame data size

//      qDebug() << "The frame:" << str.c_str() << m_socket->write(block);

//      m_socket->flush();

//      success = true;
//    }
//    catch(FailedAssertion & ae)
//    {
//      ClientRoot::getLog()->addException(ae.what());
//    }
  }

  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::checkConnected()
{
  if(!m_connectedToServer)
    ClientRoot::getLog()->addError("Not connected to the server.");

  return m_connectedToServer;
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void ClientNetworkComm::newData()
{
  QString frame;
  QDataStream in(m_socket);
  in.setVersion(QDataStream::Qt_4_5); // QDataStream version

  // if the server sends two messages very close in time, it is possible that
  // the client never gets the second one.
  // So, it is useful to explicitly read the m_socket until the end is reached.
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

    try
    {
      ClientRoot::processSignalString(frame);
    }
    catch(SignalError & se)
    {
      ClientRoot::getLog()->addException(se.what());
    }

    m_blockSize = 0;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::connectionEstablished()
{
  m_connectedToServer = true;
  emit connected();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::disconnected()
{
  if(!m_requestDisc)
  {
    ClientRoot::getLog()->addError("The connection has been closed");
  }
  emit disconnectFromServer();

  m_connectedToServer = false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ClientNetworkComm::socketError(QAbstractSocket::SocketError err)
{
  NLog::Ptr log = ClientRoot::getLog();

  if(m_requestDisc)
    return;

  if(m_connectedToServer)
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
