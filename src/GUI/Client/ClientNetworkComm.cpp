#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <string>

#include "Common/CF.hpp"
#include "Common/CPath.hpp"
#include "Common/xmlParser.h"
#include "Common/StringOps.hpp"
#include "Common/ConverterTools.hpp"
#include "Common/BuilderParser.hpp"
#include "Common/BuilderParserFrameInfo.hpp"
#include "Common/StringOps.hpp"

#include "GUI/Client/CLog.hpp"
#include "GUI/Client/ClientRoot.hpp"

#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/HostInfos.hpp"
#include "GUI/Network/NetworkException.hpp"
#include "GUI/Network/NetworkProtocol.hpp"
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
  if(!this->checkConnected())
    return false;

  return this->buildAndSend(NETWORK_CLOSE_FILE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetAbstractTypes(const QString & typeName)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_GET_ABSTRACT_TYPES);
  fi.frameAttributes["typeName"] = typeName.toStdString();
  return this->buildAndSend(fi);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetConcreteTypes(const QString & typeName)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_GET_CONCRETE_TYPES);
  fi.frameAttributes["typeName"] = typeName.toStdString();
  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendAddNode(const QDomNode & node,
                                    const QString & type,
                                    const QString & absType)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false ;

  fi.setFrameType(NETWORK_ADD_NODE);
  fi.frameAttributes["path"] = this->getNodePath(node).toStdString();
  fi.frameAttributes["type"] = type.toStdString();
  fi.frameAttributes["absType"] = absType.toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendRenameNode(const QDomNode & node,
                                       const QString & newName)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_RENAME_NODE);
  fi.frameAttributes["path"] = this->getNodePath(node).toStdString();
  fi.frameAttributes["newName"] = newName.toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendDeleteNode(const QDomNode & node)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_DELETE_NODE);
  fi.frameAttributes["path"] = this->getNodePath(node).toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendModifyNode(const QDomDocument & data)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_MODIFY_NODE);
  fi.frameData = XMLNode::parseString(data.toString().toStdString().c_str());

  return this->buildAndSend(fi);
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
    this->buildAndSend(NETWORK_SHUTDOWN_SERVER);

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
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_OPEN_FILE);
  fi.frameAttributes["filename"] = filename.toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendOpenDir(const QString & dirname,
                                    bool includeFiles,
                                    const QStringList & extensions,
                                    bool includeNoExtension)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_READ_DIR);

  fi.frameAttributes["dirPath"] = dirname.toStdString();
  fi.frameAttributes["includeFiles"] = includeFiles ? "yes" : "no";
  fi.frameAttributes["extensions"] = extensions.join("*").toStdString();
  fi.frameAttributes["includeNoExtension"] = includeNoExtension ? "yes" : "no";

  return this->buildAndSend(fi);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetTree()
{
  if(!this->checkConnected())
    return false;

  return this->buildAndSend("getTree", CLIENT_ROOT_PATH);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetHostList()
{
  if(!this->checkConnected())
    return false;

  return this->buildAndSend(NETWORK_GET_HOST_LIST);
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
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_CONFIGURE);
  fi.frameData = XMLNode::parseString(config.toString().toStdString().c_str());
  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendCreateDir(const QString & path, const QString & name)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_CREATE_DIR);
  fi.frameAttributes["path"] = path.toStdString();
  fi.frameAttributes["dirName"] = name.toStdString();
  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendSaveConfig(const QString & path, const QDomDocument & config)
{
  BuilderParserFrameInfo fi;
  QDomDocument toSend = config;

  if(!this->checkConnected())
    return false ;

  if(toSend.firstChild().nodeType() == QDomNode::ProcessingInstructionNode)
    toSend.replaceChild(toSend.childNodes().at(1), toSend.firstChild());

  fi.setFrameType(NETWORK_SAVE_CONFIG);
  fi.frameAttributes["filename"] = path.toStdString();
  fi.frameData = XMLNode::parseString(toSend.toString().toStdString().c_str());
  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendRunSimulation()
{
  if(!this->checkConnected())
    return false;

  return this->buildAndSend(NETWORK_RUN_SIMULATION);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendActivateSimulation(int nbProcs, const QString & hosts)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_ACTIVATE_SIMULATION);
  fi.frameAttributes["nbProcs"] = QString::number(nbProcs).toStdString();
  fi.frameAttributes["hosts"] = hosts.toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendDeactivateSimulation()
{
  if(!this->checkConnected())
    return false;

  return this->buildAndSend(NETWORK_DEACTIVATE_SIMULATION);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendGetSubSystemList()
{
  if(!this->checkConnected())
    return false;

  return this->buildAndSend(NETWORK_GET_SUBSYSTEM_LIST);
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
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_ADD_COMPONENT);
  fi.frameAttributes["parentPath"] = path.toStdString();
  fi.frameAttributes["type"] = ComponentType::Convert::to_str(type);
  fi.frameAttributes["name"] = name.toStdString();

  return this->buildAndSend(fi);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::sendAddLink(const QString & path,
                                    const QString & name,
                                    const QString & target)
{
  BuilderParserFrameInfo fi;

  if(!this->checkConnected())
    return false;

  fi.setFrameType(NETWORK_ADD_LINK);
  fi.frameAttributes["parentPath"] = path.toStdString();
  fi.frameAttributes["name"] = name.toStdString();
  fi.frameAttributes["target"] = target.toStdString();

  return this->buildAndSend(fi);
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

    qDebug() << str.c_str();

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

bool ClientNetworkComm::buildAndSend(const BuilderParserFrameInfo & frameInfos)
{
  bool success = false;
  std::string frame;

  try
  {
    if(!BuilderParser::buildFrame(frameInfos, m_protocol, frame))
      ClientRoot::getLog()->addError(BuilderParser::getErrorString().c_str());
    else
      success = this->send(frame.c_str()) != 0;
  }
  catch(std::string str)
  {
    ClientRoot::getLog()->addError(str.c_str());
  }
  return success;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::buildAndSend(NetworkFrameType type)
{
  BuilderParserFrameInfo fi;
  fi.setFrameType(type);
  return this->buildAndSend(fi);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ClientNetworkComm::buildAndSend(const QString & type, const CPath & sender)
{
  SignalInfo si(type, sender, SERVER_ROOT_PATH, true);

  return this->send(si);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString ClientNetworkComm::getNodePath(const QDomNode & node) const
{
  QDomNode parentNode = node.parentNode();
  QString path;

  if(parentNode.isNull()) // if the node has no parent
    return QString();
  else
  {
    path = this->getNodePath(parentNode);
    return path + '/' + node.nodeName();
  }
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
  BuilderParserFrameInfo fi;
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

    qDebug() << frame;

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
