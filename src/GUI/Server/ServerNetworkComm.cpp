#include <iostream>
#include <string>

#include <QtNetwork>
#include <QtCore>

#include "Common/StringOps.hpp"
#include "Common/XmlHelpers.hpp"

#include "Common/ConfigArgs.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/LogMessage.hpp"
#include "GUI/Network/NetworkException.hpp"

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
  QHash<QTcpSocket*, std::string>::iterator it = m_clients.begin();

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

int ServerNetworkComm::send(QTcpSocket * client, const XmlNode & signal)
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  int count = 0; // total bytes sent

  std::string signalStr;

  XmlOps::xml_to_string(signal, signalStr);

  out.setVersion(QDataStream::Qt_4_6);
  // reserving 2 bytes to store the data size
  // (frame size without these 2 bytes)
  out << (quint32)0;
  // if data is not converted to QString, the client receives q strqnge frame
  // composed of chinese/japanese chararcters
  out << QString(signalStr.c_str());
  out.device()->seek(0); // go back to the beginning of the frame
  out << (quint32)(block.size() - sizeof(quint32)); // store the data size

  if(client == CFNULL)
  {
    QHash<QTcpSocket *, std::string>::iterator it = m_clients.begin();

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

void ServerNetworkComm::sendSignalToClient(const XmlNode & signal, const string & uuid)
{
  QTcpSocket * socket = this->getSocket(uuid);
  this->send(socket, signal);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::disconnectAll()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendErrorToClient(const QString & message, const string & uuid)
{
  QTcpSocket * socket = this->getSocket(uuid);
  this->sendError(socket, message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendMessageToClient(const QString & message, const string & uuid)
{
  QTcpSocket * socket = this->getSocket(uuid);
  this->sendMessage(socket, message);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendMessage(QTcpSocket * client, const QString & message)
{
  boost::shared_ptr<XmlNode> doc = XmlOps::create_doc();
  XmlNode * signal = XmlOps::add_signal_frame(*XmlOps::goto_doc_node(*doc.get()),
                                              "message", SERVER_CORE_PATH,
                                              CLIENT_LOG_PATH, false);
  XmlParams p(*signal);

  p.add_param("type", LogMessage::Convert::to_str(LogMessage::INFO));
  p.add_param("text", message.toStdString());

  return this->send(client, *doc) != 0;
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

bool ServerNetworkComm::sendError(QTcpSocket * client, const QString & message)
{
  boost::shared_ptr<XmlNode> doc = XmlOps::create_doc();
  XmlNode * signal = XmlOps::add_signal_frame(*XmlOps::goto_doc_node(*doc.get()),
                                              "message", SERVER_CORE_PATH,
                                              CLIENT_LOG_PATH, false);
  XmlParams p(*signal);

  p.add_param("type", LogMessage::Convert::to_str(LogMessage::ERROR));
  p.add_param("text", message.toStdString());

  return this->send(client, *doc) != 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QTcpSocket * ServerNetworkComm::getSocket(const string & uuid) const
{
  QTcpSocket * socket = CFNULL;

  if(!uuid.empty())
  {
    socket = m_clients.key(uuid, CFNULL);

    if(socket == CFNULL)
      throw UnknownClientIdException(FromHere(), "Unknown client id: " + uuid);
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
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::newData()
{
  // which client has sent data ?
  QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());

  try
  {
    std::string target;
    std::string receiver;
    std::string clientId;
    QString errorMsg;
    XmlAttr * tmpNode;

    QString frame;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_6);

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

       qDebug() << frame;

      m_bytesRecieved += m_blockSize + (int)sizeof(quint32);

      boost::shared_ptr<XmlDoc> xmldoc = XmlOps::parse( frame.toStdString() );

      qDebug() << "parsed";

      XmlNode& nodedoc = *XmlOps::goto_doc_node(*xmldoc.get());
      XmlNode& frameNode = *nodedoc.first_node();

      tmpNode = frameNode.first_attribute("target");

      if(tmpNode == CFNULL)
        errorMsg += "target is missing\n";
      else
        target = tmpNode->value();

      tmpNode = frameNode.first_attribute("receiver");

      if(tmpNode == CFNULL)
        errorMsg += "reciever is missing\n";
      else
        receiver = tmpNode->value();

      tmpNode = frameNode.first_attribute("clientid");

      if(tmpNode == CFNULL)
        errorMsg += "clientid is missing\n";
      else
        clientId = tmpNode->value();

      if(!errorMsg.isEmpty())
      {
        errorMsg = QString("Illegal frame received:\n") % frame % '\n' % errorMsg;
        this->sendError(socket, errorMsg);
      }
      else
      {
        if(target == "client_registration")
        {
          if(!m_clients[socket].empty())
            this->sendError(socket, "This client has already been registered.");
          else
          {
            m_clients[socket] = clientId;

            // Build the reply
            XmlNode * replyNode = XmlOps::add_reply_frame(*nodedoc.first_node());
            XmlParams reply(*replyNode);

            reply.add_param("accepted", true);

            this->send(socket, *xmldoc.get());

            emit newClientConnected(clientId);
          }
        }
        else
        {
          if(m_clients[socket].empty())
          {
            QString msg = "The resquest has been rejected because it came "
                          "from an unregistered client.";
            this->sendError(socket, msg);
          }
          else if(m_clients[socket] != clientId)
          {
            QString msg = "The client id '%1' (used for registration) "
                          "and '%2' (used for identification) do not "
                          "match.";
            this->sendError(socket, msg.arg(m_clients[socket].c_str()).arg(clientId.c_str()));
          }
          else
            ServerRoot::processSignal(target, receiver, clientId, nodedoc);
        }
      }

      m_blockSize = 0;
    }
  }
  catch(Exception & e)
  {
    this->sendError(socket, QString("Could not execute the request because "
                                    "a CF exception has been caught: ") + e.what());
  }
  catch(std::exception & stde)
  {
    this->sendError(socket, QString("Could not execute the request because "
                                    "a std exception has been caught: ") +  stde.what());
  }
  catch(...)
  {
    this->sendError(socket, "Could not execute the request because an "
                    "unknown exception has been caught.");
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
