// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDateTime>

#include "common/Signal.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "UI/UICommon/ComponentNames.hpp"

#include "UI/Core/TreeThread.hpp"
#include "UI/Core/ThreadManager.hpp"

#include "UI/Core/NLog.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::ui::UICommon;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

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

void NLog::add_message(const QString & message)
{
  cf3_assert(!message.isEmpty());

  this->append_to_log(LogMessage::INFO, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::add_error(const QString & message)
{
  cf3_assert(!message.isEmpty());

  this->append_to_log(LogMessage::ERROR, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::add_warning(const QString & message)
{
  cf3_assert(!message.isEmpty());

  this->append_to_log(LogMessage::WARNING, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::add_exception(const QString & message)
{
  cf3_assert(!message.isEmpty());

  this->append_to_log(LogMessage::EXCEPTION, false, message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::append_to_log(LogMessage::Type type, bool fromServer,
                       const QString & message)
{
  QString header = "[ %1 ][ %2 ] ";
  QString msg;

  header = header.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
  header = header.arg(fromServer ? "Server" : "Client");

  msg = message;
  msg.replace('\n', QString("\n%1").arg(header));
  msg.prepend(header);

  emit new_message(msg, type);

  if(type == LogMessage::EXCEPTION)
    emit new_exception(message);
}

////////////////////////////////////////////////////////////////////////////

void NLog::signal_message(SignalArgs & node)
{
  SignalOptions options( node );

  std::string type_str = options.value<std::string>("type");
  std::string message = options.value<std::string>("text");
  LogMessage::Type type = LogMessage::Convert::instance().to_enum(type_str);

  cf3_assert(type != LogMessage::INVALID);

  this->append_to_log(type, true, message.c_str());
}

////////////////////////////////////////////////////////////////////////////

QString NLog::tool_tip() const
{
  return this->component_type();
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

NLog::Ptr NLog::global()
{
  static NLog::Ptr log = ThreadManager::instance().tree().root_child<NLog>(CLIENT_LOG);
  cf3_assert( is_not_null(log.get()) );
  return log;
}

//////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
