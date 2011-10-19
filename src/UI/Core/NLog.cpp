// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDateTime>

#include "Common/Signal.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NLog.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::UI::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

NLog::NLog()
  : CNode(CLIENT_LOG, "NLog", CNode::DEBUG_NODE)
{
  m_typeNames[ LogMessage::INFO ]      = "  Info   ";
  m_typeNames[ LogMessage::EXCEPTION ] = "Exception";
  m_typeNames[ LogMessage::ERROR ]     = "  Error  ";
  m_typeNames[ LogMessage::WARNING ]   = " Warning ";

  regist_signal( "message" )
    ->description("Log message")
    ->pretty_name("")->connect(boost::bind(&NLog::signal_message, this, _1));
}

////////////////////////////////////////////////////////////////////////////

NLog::~NLog()
{

}

////////////////////////////////////////////////////////////////////////////

void NLog::addMessage(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::INFO, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::addError(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::ERROR, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::addWarning(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::WARNING, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::addException(const QString & message)
{
  cf_assert(!message.isEmpty());

  this->appendToLog(LogMessage::EXCEPTION, false, message);
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

void NLog::signal_message(SignalArgs & node)
{
  SignalOptions options( node );

  std::string typeStr = options.value<std::string>("type");
  std::string message = options.value<std::string>("text");
  LogMessage::Type type = LogMessage::Convert::instance().to_enum(typeStr);

  cf_assert(type != LogMessage::INVALID);

  this->appendToLog(type, true, message.c_str());
}

////////////////////////////////////////////////////////////////////////////

QString NLog::toolTip() const
{
  return this->componentType();
}

////////////////////////////////////////////////////////////////////////////

void NLog::message (const std::string & data )
{
//  QString message(data.c_str());
//  QString typeStr = message.split(" ").first();
//  QString copy(message);
//  LogMessage::Type type = LogMessage::Convert::instance().to_enum(typeStr.toStdString());

//  this->appendToLog ( type, false, copy.remove(0, typeStr.length() + 1) );
}

////////////////////////////////////////////////////////////////////////////

NLog::Ptr NLog::globalLog()
{
  static NLog::Ptr log = ThreadManager::instance().tree().rootChild<NLog>(CLIENT_LOG);
  cf_assert( is_not_null(log.get()) );
  return log;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
