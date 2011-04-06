// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <QtNetwork>
#include <QtCore>
#include <QDebug>

#include "rapidxml/rapidxml.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"
#include "UI/UICommon/LogMessage.hpp"

#include "UI/Server/ServerExceptions.hpp"
#include "UI/Server/ServerRoot.hpp"

#include "UI/Server/ServerNetworkComm.hpp"

using namespace std;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::UI::UICommon;
using namespace CF::UI::Server;

ServerNetworkComm::ServerNetworkComm()
  : m_server(nullptr),
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

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::openPort(quint16 port)
{
  bool success = false;

  if(m_server == nullptr)
  {
    m_server = new QTcpServer(this);

    if(!m_server->listen(QHostAddress::Any, port))
    {
      QString message = QString("Cannot listen %1 on port %2 : %3")
                        .arg("")
                        .arg(port)
                        .arg(m_server->errorString());
      throw NetworkError(FromHere(), message.toStdString());
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(newClient()));
    m_server->setMaxPendingConnections(1);

    success = true;
  }

  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ServerNetworkComm::send(QTcpSocket * client, const XmlDoc & signal)
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  int count = 0; // total bytes sent

  std::string signal_str;

  XML::to_string(signal, signal_str);

  out.setVersion(QDataStream::Qt_4_6);

  out.writeBytes(signal_str.c_str(), signal_str.length() + 1);

  if(client == nullptr)
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

void ServerNetworkComm::sendSignalToClient(const XmlDoc & signal, const string & uuid)
{
  QTcpSocket * socket = this->getSocket(uuid);
  this->send(socket, signal);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendFrameRejectedToClient(const string clientid,
                                                  const string & frameid,
                                                  const URI & sender,
                                                  const QString & reason)
{
  QTcpSocket * socket = this->getSocket(clientid);
  this->sendFrameRejected(socket, frameid, sender, reason);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::disconnectAll()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::sendMessageToClient(const QString & message,
                                            LogMessage::Type type,
                                            const string & uuid)
{
  QTcpSocket * socket = this->getSocket(uuid);
  this->sendMessage(socket, message, type);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendFrameRejected(QTcpSocket * clientId,
                                          const string & frameid,
                                          const URI & sender,
                                          const QString & reason)
{
  SignalFrame frame("frame_rejected", sender, CLIENT_ROOT_PATH);
  SignalOptions options( frame );

  options.add("frameid", frameid);
  options.add("reason", reason.toStdString());

  return this->send(clientId, *frame.xml_doc.get()) != 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ServerNetworkComm::sendMessage(QTcpSocket * client, const QString & message,
                                    LogMessage::Type type)
{
  SignalFrame frame("message", SERVER_CORE_PATH, CLIENT_LOG_PATH);
  SignalOptions options( frame );


  if(type == LogMessage::INVALID)
    type = LogMessage::INFO;

  options.add("type", LogMessage::Convert::instance().to_str(type));
  options.add("text", message.toStdString());

  return this->send(client, *frame.xml_doc.get()) != 0;
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

QTcpSocket * ServerNetworkComm::getSocket(const string & uuid) const
{
  QTcpSocket * socket = nullptr;

  if(!uuid.empty())
  {
    socket = m_clients.key(uuid, nullptr);

    if(socket == nullptr)
      throw UnknownClientId(FromHere(), "Unknown client id: " + uuid);
  }

  return socket;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

string ServerNetworkComm::getAttr(const XmlNode & node, const char * paramName,
																	QString & reason)
{
	string param;

	if (reason.isEmpty())
	{
		rapidxml::xml_attribute<>* tmpAttr = node.content->first_attribute(paramName);

		if(tmpAttr == nullptr)
			reason = QString("%1 is missing.").arg(paramName);
		else
			param = tmpAttr->value();
	}

	return param;
}


/****************************************************************************

SLOTS

*****************************************************************************/

void ServerNetworkComm::newClient()
{
  QTcpSocket * socket;

  socket = m_server->nextPendingConnection();

  // connect useful signals to slots
  connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(newData()));

  std::cout << "A new client is connected" << std::endl;

  ServerRoot::listenToEvents();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::newData()
{
  // which client has sent data ?
  QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());
  std::string target;
  std::string receiver;
  std::string clientId;
  std::string frameId;

  QString errorMsg;

  try
  {
    char * frame;
    QDataStream in(socket);

    in.setVersion(QDataStream::Qt_4_6); // set stream version

    // if the client sends two messages very close in time, it is possible that
    // the server never gets the second one.
    // So, it is useful to explicitly read the socket until the end is reached.
    while(!socket->atEnd())
    {
      in.readBytes(frame, m_blockSize);

      m_bytesRecieved += m_blockSize + (int)sizeof(quint32);

      XmlDoc::Ptr xmldoc = XML::parse_cstring( frame, m_blockSize - 1 );

      // free the buffer
      delete[] frame;
      frame = nullptr;

      XmlNode nodedoc = Protocol::goto_doc_node(*xmldoc.get());
      SignalFrame * sig_frame = new SignalFrame( nodedoc.content->first_node() );

      sig_frame->xml_doc = xmldoc;

      target = this->getAttr(sig_frame->node, "target", errorMsg);
      receiver = this->getAttr(sig_frame->node, "receiver", errorMsg);
      clientId = this->getAttr(sig_frame->node, "clientid", errorMsg);
      frameId = this->getAttr(sig_frame->node, "frameid", errorMsg);

      if(errorMsg.isEmpty())
      {
        if(target == "client_registration")
        {
          if(!m_clients[socket].empty())
            errorMsg = "This client has already been registered.";
          else
          {
            m_clients[socket] = clientId;

            // Build the reply
            SignalFrame reply = sig_frame->create_reply();
            SignalOptions roptions( reply );

            roptions.add<bool>("accepted", true);

            this->send(socket, *xmldoc.get());

            emit newClientConnected(clientId);
          }
        }
        else
        {
          if(m_clients[socket].empty())
            errorMsg = "The signal came from an unregistered client.";
          else if(m_clients[socket] != clientId)
            errorMsg = QString("The client id '%1' (used for registration) "
                               "and '%2' (used for identification) do not "
                               "match.").arg(m_clients[socket].c_str()).arg(clientId.c_str());
          else
            ServerRoot::processSignal(target, receiver, clientId, frameId, *sig_frame);
        }
      }

      m_blockSize = 0;
    }
  }
  catch(Exception & e)
  {
    this->sendMessage(socket, e.what(), LogMessage::EXCEPTION);
  }
  catch(std::exception & stde)
  {
    this->sendMessage(socket, stde.what(), LogMessage::EXCEPTION);
  }
  catch(...)
  {
    errorMsg = QString("An unknown exception has been caught.");
  }

	if(!errorMsg.isEmpty())
		this->sendFrameRejected(socket, frameId, SERVER_CORE_PATH, errorMsg);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::clientDisconnected()
{
  // which client has been disconnected ?
  QTcpSocket * socket = qobject_cast<QTcpSocket *>(sender());

  if(socket != nullptr)
  {
    m_clients.remove(socket);

    std::cout << "A client has gone (" << m_clients.size() << " left)\n";
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::message(const QString & message)
{
  this->sendMessage((QTcpSocket*)NULL, message, LogMessage::INFO);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ServerNetworkComm::error(const QString & message)
{
  this->sendMessage((QTcpSocket*)NULL, message, LogMessage::ERROR);
}
