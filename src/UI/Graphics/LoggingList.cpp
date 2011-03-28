// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/NLog.hpp"

#include "UI/UICommon/LogMessage.hpp"

#include "UI/Graphics/LoggingList.hpp"

using namespace CF::UI::UICommon;
using namespace CF::UI::Core;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Graphics {

//////////////////////////////////////////////////////////////////////////

LoggingList::LoggingList(QWidget * parent, unsigned int maxLogLines)
  : QTextEdit(parent),
    m_maxLogLines(maxLogLines)
{
  qRegisterMetaType<CF::UI::UICommon::LogMessage::Type>("CF::UI::UICommon::LogMessage::Type");
  this->setWordWrapMode(QTextOption::NoWrap);
  this->setReadOnly(true);

  connect(NLog::globalLog().get(), SIGNAL(newMessage(QString,CF::UI::UICommon::LogMessage::Type)),
           this, SLOT(newMessage(QString,CF::UI::UICommon::LogMessage::Type)));
}

//////////////////////////////////////////////////////////////////////////

LoggingList::~LoggingList()
{

}

//////////////////////////////////////////////////////////////////////////

void LoggingList::setMaxLogLines(unsigned int maxLogLines)
{
  m_maxLogLines = maxLogLines;
}

//////////////////////////////////////////////////////////////////////////

unsigned int LoggingList::maxLogLines() const
{
  return m_maxLogLines;
}

//////////////////////////////////////////////////////////////////////////

unsigned int LoggingList::logLinesCount() const
{
  return m_logLinesCounter;
}

//////////////////////////////////////////////////////////////////////////

void LoggingList::clearLog()
{
  m_logLinesCounter = 0;
  this->clear();
}

 // PUBLIC SLOT

void LoggingList::newMessage(const QString & message, LogMessage::Type type)
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
} // CF
