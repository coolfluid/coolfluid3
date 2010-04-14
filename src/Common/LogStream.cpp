#include <iostream>

#include "Common/PE.hpp"
#include "Common/Log.hpp"
#include "Common/LogStream.hpp"
#include "Common/LogLevelFilter.hpp"
#include "Common/LogStampFilter.hpp"

using namespace CF;
using namespace CF::Common;
using namespace boost;

LogStream::LogStream(const std::string & streamName, LogLevel level)
: m_buffer(),
m_streamName(streamName),
m_level(level),
m_flushed(true)
{
  iostreams::filtering_ostream * stream;
  LogLevelFilter levelFilter(level);

  // SCREEN
  stream = new iostreams::filtering_ostream();
  stream->push(levelFilter);
  stream->push(LogStampFilter(streamName));
  stream->push(std::cout);
  m_destinations[SCREEN] = stream;

  // FILE
  m_destinations[FILE] = NULL;

  // STRING
  stream = new iostreams::filtering_ostream();
  stream->push(levelFilter);
  stream->push(LogStampFilter(streamName));
  stream->push(back_inserter(m_buffer));
  m_destinations[STRING] = stream;

  // SYNC_SCREEN
  stream = new iostreams::filtering_ostream();
  stream->push(levelFilter);
  stream->push(LogStampFilter(streamName));
  stream->push(std::cout);
  m_destinations[SYNC_SCREEN] = stream;


  // by default, we use all destinations
  m_usedDests[SCREEN] = true;
  m_usedDests[FILE] = true;
  m_usedDests[STRING] = true;
  m_usedDests[SYNC_SCREEN] = true;

  m_filterRankZero[SCREEN] = true;
  m_filterRankZero[FILE] = true;
  m_filterRankZero[STRING] = true;
  m_filterRankZero[SYNC_SCREEN] = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream::~LogStream()
{
  std::map<LogDestination, iostreams::filtering_ostream *>::iterator it;

  if(!m_flushed)
    this->flush();

  for(it = m_destinations.begin() ; it != m_destinations.end() ; it++)
    delete it->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & LogStream::operator << (LogLevel level)
{
  this->getLevelFilter(SCREEN).setCurrentLogLevel(level);

  if(this->isFileOpen())
    this->getLevelFilter(FILE).setCurrentLogLevel(level);

  this->getLevelFilter(STRING).setCurrentLogLevel(level);
  this->getLevelFilter(SYNC_SCREEN).setCurrentLogLevel(level);

  return *this;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & LogStream::operator << (LogTag tag)
{
  if(tag == ENDLINE)
    this->flush();

  return *this;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & LogStream::operator << (const CodeLocation & place)
{
  this->getStampFilter(SCREEN).setPlace(place);

  if(this->isFileOpen())
    this->getStampFilter(FILE).setPlace(place);

  this->getStampFilter(STRING).setPlace(place);
  this->getStampFilter(SYNC_SCREEN).setPlace(place);

  return *this;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::flush()
{
  std::map<LogDestination, iostreams::filtering_ostream *>::iterator it;

  for(it = m_destinations.begin() ; it != m_destinations.end() ; it++)
  {
    if(this->isDestinationUsed(it->first))
    {
      it->second->strict_sync();
      it->second->clear();
    }
  }

  this->getLevelFilter(SCREEN).resetToDefaultLevel();

  if(this->isFileOpen())
    this->getLevelFilter(FILE).resetToDefaultLevel();

  this->getLevelFilter(STRING).resetToDefaultLevel();
  this->getLevelFilter(SYNC_SCREEN).resetToDefaultLevel();

  this->getStampFilter(SCREEN).endMessage();

  if(this->isFileOpen())
    this->getStampFilter(FILE).endMessage();

  this->getStampFilter(STRING).endMessage();
  this->getStampFilter(SYNC_SCREEN).endMessage();

  if(!m_buffer.empty())
  {
    std::list<LogStringForwarder *>::iterator it = m_stringForwarders.begin();

    while(it != m_stringForwarders.end())
    {
      (*it)->message(m_buffer);
      it++;
    }
    m_buffer.clear();
  }

  m_flushed = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setLogLevel(LogLevel level)
{
  m_level = level;
  this->getLevelFilter(SCREEN).setLogLevel(level);

  if(this->isFileOpen())
    this->getLevelFilter(FILE).setLogLevel(level);

  this->getLevelFilter(STRING).setLogLevel(level);
  this->getLevelFilter(SYNC_SCREEN).setLogLevel(level);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setLogLevel(LogDestination destination, LogLevel level)
{
  this->getLevelFilter(destination).setLogLevel(level);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogLevel LogStream::getLogLevel(LogDestination destination) const
{

  return this->getLevelFilter(destination).getLogLevel();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::useDestination(LogDestination destination, bool use)
{
  m_usedDests[destination] = use;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool LogStream::isDestinationUsed(LogDestination destination) const
{
  if(destination == FILE && !this->isFileOpen())
    return false;

  return m_usedDests.find(destination)->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setStamp(LogDestination destination, const std::string & stampFormat)
{
  this->getStampFilter(destination).setStamp(stampFormat);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string LogStream::getStamp(LogDestination destination)
{
  if(!this->isFileOpen() && destination == LogStream::FILE)
    return "";

  return this->getStampFilter(destination).getStamp();
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setStamp(const std::string & stampFormat)
{
  this->getStampFilter(SCREEN).setStamp(stampFormat);

  if(this->isFileOpen())
    this->getStampFilter(FILE).setStamp(stampFormat);

  this->getStampFilter(STRING).setStamp(stampFormat);
  this->getStampFilter(SYNC_SCREEN).setStamp(stampFormat);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setFilterRankZero(LogDestination dest, bool filterRankZero)
{
  m_filterRankZero[dest] = filterRankZero;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool LogStream::getFilterRankZero(LogDestination dest) const
{
  return m_filterRankZero.find(dest)->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setFilterRankZero(bool filterRankZero)
{
  m_filterRankZero[SCREEN] = filterRankZero;
  m_filterRankZero[FILE] = filterRankZero;
  m_filterRankZero[STRING] = filterRankZero;
  m_filterRankZero[SYNC_SCREEN] = filterRankZero;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::setFile(const iostreams::file_descriptor_sink & fileDescr)
{
  if(!this->isFileOpen())
  {
    iostreams::filtering_ostream * stream = new iostreams::filtering_ostream();

    stream->push(LogLevelFilter(m_level));
    stream->push(LogStampFilter(m_streamName));
    stream->push(fileDescr);

    m_destinations[FILE] = stream;
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogLevelFilter & LogStream::getLevelFilter(LogDestination dest) const
{
  return *m_destinations.find(dest)->second->component<LogLevelFilter>(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStampFilter & LogStream::getStampFilter(LogDestination dest) const
{
  return *m_destinations.find(dest)->second->component<LogStampFilter>(1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool LogStream::isFileOpen() const
{
  return m_destinations.find(FILE)->second != NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::addStringForwarder(LogStringForwarder * forwarder)
{
  std::list<LogStringForwarder *>::iterator begin = m_stringForwarders.begin();
  std::list<LogStringForwarder *>::iterator end = m_stringForwarders.end();

  if(forwarder != NULL && std::find(begin, end, forwarder) == end)
    m_stringForwarders.push_back(forwarder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogStream::removeStringForwarder(LogStringForwarder * forwarder)
{
  m_stringForwarders.remove(forwarder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int LogStream::getStringForwarderCount() const
{
  return m_stringForwarders.size();
}

