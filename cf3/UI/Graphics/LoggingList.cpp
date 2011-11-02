// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NLog.hpp"

#include "UI/UICommon/LogMessage.hpp"

#include "UI/Graphics/LoggingList.hpp"

using namespace cf3::ui::UICommon;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

LoggingList::LoggingList(QWidget * parent, unsigned int maxLogLines)
  : QTextEdit(parent),
    m_max_log_lines(maxLogLines)
{
  qRegisterMetaType<LogMessage::Type>("UICommon::LogMessage::Type");
  this->setWordWrapMode(QTextOption::NoWrap);
  this->setReadOnly(true);

  connect(NLog::global().get(), SIGNAL(new_message(QString, UICommon::LogMessage::Type)),
           this, SLOT(new_message(QString, UICommon::LogMessage::Type)));
}

//////////////////////////////////////////////////////////////////////////

LoggingList::~LoggingList()
{

}

//////////////////////////////////////////////////////////////////////////

void LoggingList::set_max_log_lines(unsigned int maxLogLines)
{
  m_max_log_lines = maxLogLines;
}

//////////////////////////////////////////////////////////////////////////

unsigned int LoggingList::max_log_lines() const
{
  return m_max_log_lines;
}

//////////////////////////////////////////////////////////////////////////

unsigned int LoggingList::log_lines_count() const
{
  return m_log_lines_count;
}

//////////////////////////////////////////////////////////////////////////

void LoggingList::clear_log()
{
  m_log_lines_count = 0;
  this->clear();
}

 // PUBLIC SLOT

void LoggingList::new_message(const QString & message, LogMessage::Type type)
{
  QString msgToAppend = "<font face=\"monospace\" color=\"%1\">%2</font>";
  QString imgTag = "<img src=\":/Icons/%1.png\" height=\"%2\" width=\"%2\"> ";
  QString color;
  QString msg = message;
  QString typeName = LogMessage::Convert::instance().to_str(type).c_str();
  int size = 14;

  msg.replace(" ", "&nbsp;");
  msg.replace("<", "&lt;");
  msg.replace(">", "&gt;");

  if(type == LogMessage::ERROR || type == LogMessage::EXCEPTION)
    color = "red";
  else
    color = "black";

  imgTag = imgTag.arg(typeName).arg(size);

  msg.prepend(imgTag);

  this->append(msgToAppend.arg(color).arg(msg.replace("\n", "<br>" + imgTag)));
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3
