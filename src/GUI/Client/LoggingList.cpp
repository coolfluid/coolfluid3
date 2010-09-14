#include "Common/OptionT.hpp"

#include "GUI/Client/ClientRoot.hpp"
#include "GUI/Client/NLog.hpp"

#include "GUI/Client/LoggingList.hpp"

using namespace CF::GUI::Client;

LoggingList::LoggingList(QWidget * parent, unsigned int maxLogLines)
  : QTextEdit(parent),
    m_maxLogLines(maxLogLines)
{
  this->setWordWrapMode(QTextOption::NoWrap);
  this->setReadOnly(true);

  connect(ClientRoot::log().get(), SIGNAL(newMessage(const QString &, bool)),
           this, SLOT(newMessage(const QString &, bool)));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LoggingList::~LoggingList()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LoggingList::setMaxLogLines(unsigned int maxLogLines)
{
  m_maxLogLines = maxLogLines;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned int LoggingList::getMaxLogLines() const
{
  return m_maxLogLines;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned int LoggingList::getLogLinesCounter() const
{
  return m_logLinesCounter;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LoggingList::clearLog()
{
  m_logLinesCounter = 0;
  this->clear();
}

 // PUBLIC SLOT

void LoggingList::newMessage(const QString & message, bool isError)
{
  QString msgToAppend = "<font face=\"monospace\" color=\"%1\">%2</font>";
  QString color = isError ? "red" : "black";
  QString msg = message;

  msg.replace(" ", "&nbsp;");
  msg.replace("<", "&lt;");
  msg.replace(">", "&gt;");

  this->append(msgToAppend.arg(color).arg(msg.replace("\n", "<br>")));
}

