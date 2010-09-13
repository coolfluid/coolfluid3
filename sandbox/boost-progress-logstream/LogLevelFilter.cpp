#include "LogLevel.hpp"
#include "LogLevelFilter.hpp"

using namespace CF;
using namespace CF::Common;

LogLevelFilter::LogLevelFilter(LogLevel level)
: m_logLevel(level),
m_currentLogLevel(level)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::setLogLevel(LogLevel level)
{
  m_logLevel = level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogLevel LogLevelFilter::getLogLevel() const
{
  return m_logLevel;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::setCurrentLogLevel(LogLevel level)
{
  m_currentLogLevel = level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogLevel LogLevelFilter::getCurrentLogLevel() const
{
  return m_currentLogLevel;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogLevelFilter::resetToDefaultLevel()
{
  m_currentLogLevel = m_logLevel;
}
