#include <QtCore>

#include <string>

#include "Common/CF.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Network/ComponentNames.hpp"
#include "GUI/Network/SignalInfo.hpp"

#include "GUI/Client/CLog.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;
using namespace CF::GUI::Client;

CLog::CLog()
  : Component(CLIENT_LOG)
{
 m_typeNames[ LogMessage::INFO ]      = "  Info   ";
 m_typeNames[ LogMessage::EXCEPTION ] = "Exception";
 m_typeNames[ LogMessage::ERROR ]     = "  Error  ";
 m_typeNames[ LogMessage::WARNING ]   = " Warning ";

 regist_signal("message", "Log message")->connect(boost::bind(&CLog::message, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CLog::~CLog()
{
  qDebug() << "in CLog destructor";
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CLog::addMessage(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::INFO, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CLog::addError(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::ERROR, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CLog::addException(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::EXCEPTION, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CLog::appendToLog(LogMessage::Type type, bool fromServer,
                       const QString & message)
{
  QString header = "[ %1 ][ %2 ][ %3 ] ";
  QString msg;

  header = header.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
  header = header.arg(m_typeNames[type]);
  header = header.arg(fromServer ? "Server" : "Client");

  msg = header;
  msg += message.split("\n", QString::SkipEmptyParts).join(QString("\n") + header);

  emit newMessage(msg, type == LogMessage::ERROR || type == LogMessage::EXCEPTION);

  if(type == LogMessage::EXCEPTION)
    emit newException(message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Signal::return_t CLog::message(Signal::arg_t & node)
{
  XMLParams p(node);
  std::string typeStr = p.value<std::string>("type");
  std::string message = p.value<std::string>("text");
  LogMessage::Type type = LogMessage::Convert::to_enum(typeStr);

  cf_assert(type != LogMessage::INVALID);

  this->appendToLog(type, true, message.c_str());
 //return XMLNode::createTopXMLNode();
}
