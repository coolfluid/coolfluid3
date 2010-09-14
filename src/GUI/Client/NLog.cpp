#include <QDateTime>
#include <QFileIconProvider>

#include <string>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/NLog.hpp"

using namespace CF::Common;
using namespace CF::GUI::Network;
using namespace CF::GUI::Client;

NLog::NLog()
  : CNode(CLIENT_LOG, "NLog", CNode::LOG_NODE)
{
  BUILD_COMPONENT;

  m_typeNames[ LogMessage::INFO ]      = "  Info   ";
  m_typeNames[ LogMessage::EXCEPTION ] = "Exception";
  m_typeNames[ LogMessage::ERROR ]     = "  Error  ";
  m_typeNames[ LogMessage::WARNING ]   = " Warning ";

  regist_signal("message", "Log message")->connect(boost::bind(&NLog::message, this, _1));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NLog::~NLog()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::addMessage(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::INFO, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::addError(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::ERROR, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::addWarning(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::WARNING, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::addException(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::EXCEPTION, false, message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::appendToLog(LogMessage::Type type, bool fromServer,
                       const QString & message)
{
  QString header = "[ %1 ][ %2 ] ";
  QString msg;

  header = header.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
  header = header.arg(fromServer ? "Server" : "Client");

  msg = message;
  msg.replace('\n', QString("\n%1").arg(header));
  msg.prepend(header);

  emit newMessage(msg, type);

  if(type == LogMessage::EXCEPTION)
    emit newException(message);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NLog::message(XmlNode & node)
{
  XmlParams p(node);

  std::string typeStr = p.get_option<std::string>("type");
  std::string message = p.get_option<std::string>("text");
  LogMessage::Type type = LogMessage::Convert::to_enum(typeStr);

  cf_assert(type != LogMessage::INVALID);

  this->appendToLog(type, true, message.c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QIcon NLog::getIcon() const
{
  return QFileIconProvider().icon(QFileIconProvider::Folder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLog::getToolTip() const
{
  return this->getComponentType();
}
