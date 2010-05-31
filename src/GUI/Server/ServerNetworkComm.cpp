#include <iostream>
#include <string>

#include <QtNetwork>
#include <QtXml>
#include <QtCore>

#include "Common/xmlParser.h"
#include "Common/StringOps.hpp"

#include "Common/ConfigArgs.hpp"
#include "Common/ConverterTools.hpp"
#include "Common/BuilderParser.hpp"
#include "Common/BuilderParserFrameInfo.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/LogMessage.hpp"
#include "GUI/Network/NetworkException.hpp"
#include "GUI/Network/SignalInfo.hpp"

#include "GUI/Server/UnknownClientIdException.hpp"
#include "GUI/Server/CSimulator.hpp"
#include "GUI/Server/ServerRoot.hpp"

#include "GUI/Server/ServerNetworkComm.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::GUI::Network;
using namespace CF::GUI::Server;

ServerNetworkComm::ServerNetworkComm()
  : m_server(CFNULL),
    m_localSocket(CFNULL),
    m_lastClientId(0)
{
  m_blockSize = 0;
  m_bytesRecieved = 0;
  m_bytesSent = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ServerNetworkComm::~ServerNetworkComm()
{
  QHash<QTcpSocket*, QDomNode>::iterator it = m_clients.begin();

  while(it != m_clients.end())
  {
    delete it.key();
    it++;
  }

  m_clients.clear();

  m_server->close();
  delete m_server;

  if(m_localSocket != CFNULL)
  {
    m_localSocket->close();
    delete m_localSocket;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::openPort(const QString & hostAddress, quint16 port)
{
  bool success = false;
  bool local = hostAddress == "127.0.0.1" || hostAddress == "localhost";

  if(m_server == CFNULL)
  {
    m_server = new QTcpServer(this);

    if(!local)
      m_localSocket = new QTcpServer(this);

    if(!m_server->listen(QHostAddress(hostAddress), port))
    {
      QString message = QString("Cannot listen %1 on port %2 : %3")
                        .arg(hostAddress)
                        .arg(port)
                        .arg(m_server->errorString());
      throw NetworkException(FromHere(), message.toStdString());
    }

    if(!local && !m_localSocket->listen(QHostAddress("127.0.0.1"), port))
    {
      QString message = QString("Cannot listen 127.0.0.1 on port %2 : %3")
                        .arg(port)
                        .arg(m_server->errorString());
      throw NetworkException(FromHere(), message.toStdString());
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(newClient()));
    m_server->setMaxPendingConnections(1);

    if(!local)
    {
      connect(m_localSocket, SIGNAL(newConnection()), this, SLOT(newClient()));
      m_localSocket->setMaxPendingConnections(1);
    }

    success = true;
  }

  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::buildAndSend(QTcpSocket * client, const BuilderParserFrameInfo & frameInfos)
{
  bool success = false;
  std::string frame;

  if(!BuilderParser::buildFrame(frameInfos, m_networkProtocol, frame))
    std::cerr << "Could not build the frame: " << BuilderParser::getErrorString() << frameInfos.frameType << std::endl;
  else
    success = this->send(client, frame.c_str()) != 0;

  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ServerNetworkComm::send(QTcpSocket * client, const QString & frame)
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  int count = 0; // total bytes sent

  out.setVersion(QDataStream::Qt_4_5);
  // reserving 2 bytes to store the data size
  // (frame size without these 2 bytes)
  out << (quint32)0;
  out << frame;
  out.device()->seek(0); // go back to the beginning of the frame
  out << (quint32)(block.size() - sizeof(quint32)); // store the data size

  if(client == CFNULL)
  {
    QHash<QTcpSocket *, QDomNode>::iterator it = m_clients.begin();

    while(it != m_clients.end())
    {
      client = it.key();
      count += client->write(block);
      m_bytesSent += count;
      client->flush();
      it++;
    }
  }

  else
  {
    count = client->write(block);
    m_bytesSent += count;

    client->flush();
  }

  return count;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendDirContents(int clientId, const QString & path,
                                        const QStringList & dirs,
                                        const QStringList & files)
{
  bool success = false;

  try
  {
    BuilderParserFrameInfo fi;
    QTcpSocket * socket = this->getSocket(clientId);

    fi.setFrameType(NETWORK_DIR_CONTENTS);
    fi.frameAttributes["dirPath"] = path.toStdString();
    fi.frameAttributes["dirs"] = dirs.join("*").toStdString();
    fi.frameAttributes["files"] = files.join("*").toStdString();

    success = this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }

  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendAbstractTypes(int clientId, const QString & typeName,
                                          const QStringList & typeList)
{
  bool success = false;

  try
  {
    BuilderParserFrameInfo fi;
    QTcpSocket * socket = this->getSocket(clientId);

    fi.setFrameType(NETWORK_ABSTRACT_TYPES);
    fi.frameAttributes["typeName"] = typeName.toStdString();
    fi.frameAttributes["typeList"] = typeList.join(", ").toStdString();

    success = this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }

  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendConcreteTypes(int clientId, const QString & typeName,
                                          const QStringList & typeList)
{
  bool success = false;

  try
  {
    BuilderParserFrameInfo fi;
    QTcpSocket * socket = this->getSocket(clientId);

    fi.setFrameType(NETWORK_CONCRETE_TYPES);
    fi.frameAttributes["typeName"] = typeName.toStdString();
    fi.frameAttributes["typeList"] = typeList.join(", ").toStdString();


    success = this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }

  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendHostList(int clientId,
                                     const QList<CF::GUI::Network::HostInfos> & hosts)
{
  bool success = false;
  QList<CF::GUI::Network::HostInfos>::const_iterator it = hosts.begin();

  try
  {
    BuilderParserFrameInfo fi;
    QTcpSocket * socket = this->getSocket(clientId);
    QDomDocument doc;

    fi.setFrameType(NETWORK_HOST_LIST);

    while(it != hosts.end())
    {
      QDomElement node = doc.createElement("host");

      node.setAttribute("name", it->m_hostname);
      node.setAttribute("nbSlots", it->m_nbSlots);

      if(it->m_maxSlots != 0)
        node.setAttribute("maxSlots", it->m_maxSlots);

      doc.appendChild(node);
      it++;
    }

    fi.frameData = XMLNode::parseString(doc.toString().toStdString().c_str());

    success = this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }

  return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendTree(int clientId, const QDomDocument & tree)
{
  this->sendTree(clientId, XMLNode::parseString(tree.toString().toStdString().c_str()));
  //  try
  //  {
  //   QTcpSocket * m_socket = this->getSocket(clientId);
  //   BuilderParserFrameInfo fi;
  //
  //   fi.setFrameType(NETWORK_TREE);
  //   fi.frameData = XMLNode::parseString(tree.toString().toStdString().c_str());;
  //
  //   this->buildAndSend(m_socket, fi);
  //
  //  }
  //  catch(UnknownClientIdException e)
  //  {
  //   qDebug() << e.what();
  //  }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendTree(int clientId, const XMLNode & tree)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);
    BuilderParserFrameInfo fi;

    fi.setFrameType(NETWORK_TREE);
    fi.frameData = tree;

    this->buildAndSend(socket, fi);

  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::send(int clientId, const QString & frame)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);

    this->send(socket, frame);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendError(int clientId, const QString & message)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);
    this->sendError(socket, message);

  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendAck(int clientId, bool success, NetworkFrameType type)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);
    this->sendAck(socket, success, type);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendMessage(int clientId, const QString & message)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);

    this->sendMessage(socket, message);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendStatus(int clientId,const QString & subSysName,
                                   int rank, const QString & status)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);
    BuilderParserFrameInfo fi;

    fi.setFrameType(NETWORK_SIMULATION_STATUS);
    fi.frameAttributes["subSysName"] = subSysName.toStdString();
    fi.frameAttributes["workerRank"] = StringOps::to_str<int>(rank);
    fi.frameAttributes["value"] = status.toStdString();

    this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendSubSystemList(int clientId,
                                          const QStringList & subSystemList)
{
  try
  {
    QTcpSocket * socket = this->getSocket(clientId);
    BuilderParserFrameInfo fi;

    fi.setFrameType(NETWORK_SUBSYSTEM_LIST);
    fi.frameAttributes["subSystems"] = subSystemList.join(" ").toStdString();

    this->buildAndSend(socket, fi);
  }
  catch(UnknownClientIdException e)
  {
    qDebug() << e.what();
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendMessage(QTcpSocket * client, const QString & message)
{
//   BuilderParserFrameInfo fi;
//
//   fi.setFrameType(NETWORK_MESSAGE);
//   fi.frameAttributes["value"] = message.toStdString();

  SignalInfo si("message", SERVER_ROOT_PATH, CLIENT_LOG_PATH, true);
  QString str;

  si.setParam("type", LogMessage::Convert::to_str(LogMessage::INFO));
  si.setParam("text", message.toStdString());

  str = si.getString();

  this->send(client, str);

  return true;//this->buildAndSend(client, fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ServerNetworkComm::getBytesRecieved() const
{
  return m_bytesRecieved;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ServerNetworkComm::getBytesSent() const
{
  return m_bytesSent;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendError(QTcpSocket * client, const QString & message)
{
  BuilderParserFrameInfo fi;

  fi.setFrameType(NETWORK_ERROR);
  fi.frameAttributes["value"] = message.toStdString();

  this->buildAndSend(client, fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendAck(QTcpSocket * client, bool success, NetworkFrameType type)
{
  BuilderParserFrameInfo fi;

  if(success)
    fi.setFrameType(NETWORK_ACK);
  else
    fi.setFrameType(NETWORK_NACK);

  fi.frameAttributes["type"] = m_networkProtocol.getTypeName(type);

  this->buildAndSend(client, fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTcpSocket * ServerNetworkComm::getSocket(int clientId) const
{
  QTcpSocket * socket = CFNULL;

  if(clientId != -1)
  {
    if(m_clientIds.contains(clientId))
      socket = m_clientIds[clientId];

    else
      throw UnknownClientIdException(FromHere(), QString("Unknown client id: %1")
                                     .arg(clientId).toStdString());
  }

  return socket;
}

/****************************************************************************

SLOTS

*****************************************************************************/

void ServerNetworkComm::newClient()
{
  QTcpSocket * socket;

  socket = m_server->nextPendingConnection();

  if(socket == CFNULL)
    socket = m_localSocket->nextPendingConnection();

  // connect useful signals to slots
  connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(newData()));

  std::cout << "A new client is connected" << std::endl;

  m_clients[socket] = QDomNode();
  m_clientIds[m_lastClientId] = socket;

  emit newClient(m_lastClientId);

  m_lastClientId++;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::newData()
{
  // which client has sent data ?
  QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());
  BuilderParserFrameInfo fi;

  // unused // int clientId;

  QString frame;
  QDataStream in(socket);
  in.setVersion(QDataStream::Qt_4_5);

  // if the client sends two messages very close in time, it is possible that
  // the server never gets the second one.
  // So, it is useful to explicitly read the m_socket until the end is reached.
  while(!socket->atEnd())
  {
    // if the data size is not known
    if (m_blockSize == 0)
    {
      // if there are at least 4 bytes to read...
      if (socket->bytesAvailable() < (int)sizeof(quint32))
        return;

      // ...we read them
      in >> m_blockSize;
    }

    if (socket->bytesAvailable() < m_blockSize)
      return;

    in >> frame;

    m_bytesRecieved += m_blockSize + (int)sizeof(quint32);

    QDomDocument doc;

    doc.setContent(frame);

    ServerRoot::processSignal(doc);

    m_blockSize = 0;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::clientDisconnected()
{
  // which client has been disconnected ?
  QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());

  if(socket != CFNULL)
  {
    int clientId = m_clientIds.key(socket);
    m_clientIds.remove(clientId);
    m_clients.remove(socket);

    std::cout << "A client has gone (" << m_clients.size() << " left)\n";
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::message(const QString & message)
{
  this->sendMessage((QTcpSocket*)NULL, message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::error(const QString & message)
{
  this->sendError((QTcpSocket*)NULL, message);
}
