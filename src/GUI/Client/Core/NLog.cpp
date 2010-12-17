// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDateTime>
#include <QFileIconProvider>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/CF.hpp"
#include "Common/XmlHelpers.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Network/ComponentNames.hpp"

#include "GUI/Client/Core/NLog.hpp"

using namespace boost::assign; // for operator+=()

using namespace CF::Common;
using namespace CF::GUI::Network;
using namespace CF::GUI::ClientCore;

NLog::NLog()
  : CNode(CLIENT_LOG, "NLog", CNode::LOG_NODE)
{
   

  m_typeNames[ LogMessage::INFO ]      = "  Info   ";
  m_typeNames[ LogMessage::EXCEPTION ] = "Exception";
  m_typeNames[ LogMessage::ERROR ]     = "  Error  ";
  m_typeNames[ LogMessage::WARNING ]   = " Warning ";

  regist_signal("message", "Log message")->connect(boost::bind(&NLog::message, this, _1));

  Option::Ptr option;
  std::vector<std::string> vect;

  option = m_properties.add_option< OptionT<std::string> >("MonthsOne", "Month names", std::string("January"));

    option->restricted_list() += std::string("March"),
                                 std::string("April"),
                                 std::string("May"),
                                 std::string("June"),
                                 std::string("July"),
                                 std::string("August"),
                                 std::string("Septemeber"),
                                 std::string("October"),
                                 std::string("November"),
                                 std::string("December");
    option->mark_basic();


  vect.push_back("January");
  vect.push_back("February");

  option = m_properties.add_option< OptionArrayT<std::string> >("MonthsTwo", "Month names", vect);

  option->restricted_list() += std::string("March"),
                               std::string("April"),
                               std::string("May"),
                               std::string("June"),
                               std::string("July"),
                               std::string("August"),
                               std::string("Septemeber"),
                               std::string("October"),
                               std::string("November"),
                               std::string("December");

  option->mark_basic();
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
  LogMessage::Type type = LogMessage::Convert::instance().to_enum(typeStr);

  cf_assert(type != LogMessage::INVALID);

  this->appendToLog(type, true, message.c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLog::toolTip() const
{
  return this->getComponentType();
}
