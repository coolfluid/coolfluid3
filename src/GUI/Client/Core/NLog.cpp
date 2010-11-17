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
  BUILD_COMPONENT;

  m_typeNames[ LogMessage::INFO ]      = "  Info   ";
  m_typeNames[ LogMessage::EXCEPTION ] = "Exception";
  m_typeNames[ LogMessage::ERROR ]     = "  Error  ";
  m_typeNames[ LogMessage::WARNING ]   = " Warning ";

  regist_signal("message", "Log message")->connect(boost::bind(&NLog::message, this, _1));

  Option::Ptr option;

  option = m_property_list.add_option< OptionT<std::string> >("Months", "Month names", std::string("January"));

  option->restricted_list() += std::string("February"),
                               std::string("March"),
                               std::string("April"),
                               std::string("May"),
                               std::string("June"),
                               std::string("July"),
                               std::string("August"),
                               std::string("Septemeber"),
                               std::string("October"),
                               std::string("November"),
                               std::string("December");

  option->change_value(std::string("October"));

  option->mark_basic();

  //----------------------------------------------------

  option = m_property_list.add_option< OptionT<Uint> >("Days", "Days of the month", Uint(1));

  option->restricted_list() += Uint(2), Uint(3), Uint(4), Uint(5), Uint(6),
                               Uint(7), Uint(8), Uint(9), Uint(10), Uint(11),
                               Uint(12), Uint(13), Uint(14), Uint(15), Uint(16),
                               Uint(17), Uint(18), Uint(19), Uint(20), Uint(21),
                               Uint(22), Uint(23), Uint(24), Uint(25), Uint(26),
                               Uint(27), Uint(28), Uint(29), Uint(30), Uint(31);
  option->mark_basic();

  //----------------------------------------------------

  option = m_property_list.add_option< OptionT<Real> >("SomeReals", "Some real values", Real(1.5));

  option->restricted_list() += Real(3.141592), Real(.5772156649015328606065),
                               Real(4.660299067185320382046620161),
                               Real(0.00000000000000000234);
  option->mark_basic();

  //----------------------------------------------------

  option = m_property_list.add_option< OptionT<bool> >("ABool", "A boolean value", bool(true));

  option->restricted_list() += bool(false);
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
  LogMessage::Type type = LogMessage::Convert::to_enum(typeStr);

  cf_assert(type != LogMessage::INVALID);

  this->appendToLog(type, true, message.c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NLog::toolTip() const
{
  return this->getComponentType();
}
