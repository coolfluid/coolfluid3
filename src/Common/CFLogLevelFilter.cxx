#include "CFLogLevel.hh"
#include "CFLogLevelFilter.hh"

using namespace CF;
using namespace CF::Common;

CFLogLevelFilter::CFLogLevelFilter(CFLogLevel level)
 : m_logLevel(level),
   m_currentLogLevel(level)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogLevelFilter::setLogLevel(CFLogLevel level)
{
 m_logLevel = level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogLevel CFLogLevelFilter::getLogLevel() const
{
 return m_logLevel;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogLevelFilter::setCurrentLogLevel(CFLogLevel level)
{
 m_currentLogLevel = level;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogLevel CFLogLevelFilter::getCurrentLogLevel() const
{
 return m_currentLogLevel;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogLevelFilter::resetToDefaultLevel()
{
 m_currentLogLevel = m_logLevel;
}