#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <string>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"
#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"

#include "GUI/Client/CLog.hpp"
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

bool ClientNetworkComm::sendCloseFile()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendCloseFile");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetAbstractTypes(const QString & typeName)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendGetAbstractType");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetConcreteTypes(const QString & typeName)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendGetConcreteType");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendAddNode(const QDomNode & node,
                                    const QString & type,
                                    const QString & absType)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendAddNode");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendRenameNode(const QDomNode & node,
                                       const QString & newName)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendRenameNode");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendDeleteNode(const QDomNode & node)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendDeleteNode");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendModifyNode(const QDomDocument & data)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendModifyNode");
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

bool ClientNetworkComm::sendOpenFile(const QString & filename)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendOpenFile");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendOpenDir(const QString & dirname,
                                    bool includeFiles,
                                    const QStringList & extensions,
                                    bool includeNoExtension)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendOpenDir");
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetTree()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendGetTree");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetHostList()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendGetHostList");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::isConnected() const
{
  return m_connectedToServer;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendConfig(const QDomDocument & config)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendConfig");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendCreateDir(const QString & path, const QString & name)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendCreateDir");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendSaveConfig(const QString & path, const QDomDocument & config)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendSaveConfig");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendRunSimulation()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendRunSimulation");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendActivateSimulation(int nbProcs, const QString & hosts)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendActivateSimulation");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendDeactivateSimulation()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendDeactivateSimulation");

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetSubSystemList()
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendGetSubSystemList");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

bool ClientNetworkComm::sendAddComponent(const QString & path,
                                         ComponentType::Type type,
                                         const QString & name)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendAddComponent");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendAddLink(const QString & path,
                                    const QString & name,
                                    const QString & target)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::sendAddLink");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::send(const SignalInfo & signal)
{
  bool success = false;
 QString str;

 if(this->checkConnected())
 {
   try
   {
     QByteArray block;
     QDataStream out(&block, QIODevice::WriteOnly);

     str = signal.getString();

     out.setVersion(QDataStream::Qt_4_5); // QDataStream version
     out << (quint32)0;    // reserve 32 bits for the frame data size
     out << str;
     out.device()->seek(0);  // go back to the beginning of the frame
     out << (quint32)(block.size() - sizeof(quint32)); // write the frame data size

     m_socket->write(block);
     m_socket->flush();

     success = true;
   }
   catch(FailedAssertion & ae)
   {
     ClientRoot::getLog()->addException(ae.what());
   }
 }

 return success;
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

//bool ClientNetworkComm::buildAndSend(const BuilderParserFrameInfo & frameInfos)
//{
//  bool success = false;
//  std::string frame;

//  try
//  {
//    if(!BuilderParser::buildFrame(frameInfos, m_protocol, frame))
//      ClientRoot::getLog()->addError(BuilderParser::getErrorString().c_str());
//    else
//      success = this->send(frame.c_str()) != 0;
//  }
//  catch(std::string str)
//  {
//    ClientRoot::getLog()->addError(str.c_str());
//  }
//  return success;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool ClientNetworkComm::buildAndSend(NetworkFrameType type)
//{
//  BuilderParserFrameInfo fi;
//  fi.setFrameType(type);
//  return this->buildAndSend(fi);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::buildAndSend(const QString & type, const CPath & sender)
{
  throw NotImplemented(FromHere(), "ClientNetworkComm::buildAndSend");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//QString ClientNetworkComm::getNodePath(const QDomNode & node) const
//{
//  QDomNode parentNode = node.parentNode();
//  QString path;

//  if(parentNode.isNull()) // if the node has no parent
//    return QString();
//  else
//  {
//    path = this->getNodePath(parentNode);
//    return path + '/' + node.nodeName();
//  }
//}

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
  CLog::Ptr log = ClientRoot::getLog();

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
