#include <iostream>

#include <mpi.h>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "PE.hh"
#include "CFLog.hh"
#include "CFLogStream.hh"
#include "CFLogLevelFilter.hh"
#include "CFLogStampFilter.hh"

using namespace MPI;
using namespace CF;
using namespace CF::Common;
using namespace boost;

CFLogStream::CFLogStream(const std::string & streamName, CFLogLevel level) 
 : m_flushed(true), 
   m_buffer(),
   m_streamName(streamName),
   m_level(level)
{
 iostreams::filtering_ostream * stream;
 CFLogLevelFilter levelFilter(level);
  
 // SCREEN
 stream = new iostreams::filtering_ostream();
 stream->push(levelFilter);
 stream->push(CFLogStampFilter(streamName));
 stream->push(std::cout);
 this->m_destinations[SCREEN] = stream;
 
 // FILE
 this->m_destinations[FILE] = NULL;

 // STRING
 stream = new iostreams::filtering_ostream();
 stream->push(levelFilter);
 stream->push(CFLogStampFilter(streamName));
 stream->push(back_inserter(m_buffer));
 this->m_destinations[STRING] = stream;
 
 // SYNC_SCREEN
 stream = new iostreams::filtering_ostream();
 stream->push(levelFilter);
 stream->push(CFLogStampFilter(streamName));
 stream->push(std::cout);
 this->m_destinations[SYNC_SCREEN] = stream;
 
 
 // by default, we use all destinations
 this->m_usedDests[SCREEN] = true;
 this->m_usedDests[FILE] = true;
 this->m_usedDests[STRING] = true;
 this->m_usedDests[SYNC_SCREEN] = true;
 
//  this->setFilterRankZero(tr
 this->m_filterRankZero[SCREEN] = true;
 this->m_filterRankZero[FILE] = true;
 this->m_filterRankZero[STRING] = true;
 this->m_filterRankZero[SYNC_SCREEN] = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogStream::~CFLogStream()
{
 std::map<CFLogDestination, iostreams::filtering_ostream *>::iterator it;
 
 if(!this->m_flushed)
  this->flush();
 
 for(it = m_destinations.begin() ; it != m_destinations.end() ; it++)
  delete it->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogStream & CFLogStream::operator << (CFLogLevel level)
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

CFLogStream & CFLogStream::operator << (LogTag tag) 
{
 if(tag == ENDLINE)
  this->flush();

 return *this;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogStream & CFLogStream::operator << (const CodeLocation & place)
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

void CFLogStream::flush()
{ 
 std::map<CFLogDestination, iostreams::filtering_ostream *>::iterator it;
 
 for(it = this->m_destinations.begin() ; it != this->m_destinations.end() ; it++)
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
 
 if(!this->m_buffer.empty())
 {
  std::list<CFLogStringForwarder *>::iterator it = m_stringForwarders.begin();
  
  while(it != m_stringForwarders.end())
  {
   (*it)->message(m_buffer);
   it++;
  }
  this->m_buffer.clear();
 }
 
 this->m_flushed = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setLogLevel(CFLogLevel level)
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

void CFLogStream::setLogLevel(CFLogDestination destination, CFLogLevel level)
{
 this->getLevelFilter(destination).setLogLevel(level);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogLevel CFLogStream::getLogLevel(CFLogDestination destination) const
{
 
 return this->getLevelFilter(destination).getLogLevel();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::useDestination(CFLogDestination destination, bool use)
{
 this->m_usedDests[destination] = use;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CFLogStream::isDestinationUsed(CFLogDestination destination) const
{
 if(destination == FILE && !this->isFileOpen())
  return false;
 
 return this->m_usedDests.find(destination)->second;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setStamp(CFLogDestination destination, const std::string & stampFormat)
{
 this->getStampFilter(destination).setStamp(stampFormat);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string CFLogStream::getStamp(CFLogDestination destination)
{
 return this->getStampFilter(destination).getStamp();
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setStamp(const std::string & stampFormat)
{
 this->getStampFilter(SCREEN).setStamp(stampFormat);
 
 if(this->isFileOpen())
  this->getStampFilter(FILE).setStamp(stampFormat);
 
 this->getStampFilter(STRING).setStamp(stampFormat);
 this->getStampFilter(SYNC_SCREEN).setStamp(stampFormat);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setFilterRankZero(CFLogDestination dest, bool filterRankZero)
{
 this->m_filterRankZero[dest] = filterRankZero;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setFilterRankZero(bool filterRankZero)
{ 
 this->m_filterRankZero[SCREEN] = filterRankZero;
 this->m_filterRankZero[FILE] = filterRankZero;
 this->m_filterRankZero[STRING] = filterRankZero;
 this->m_filterRankZero[SYNC_SCREEN] = filterRankZero;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::setFile(const iostreams::file_descriptor_sink & fileDescr)
{
 if(!this->isFileOpen())
 {
  iostreams::filtering_ostream * stream = new iostreams::filtering_ostream();
  
  stream->push(CFLogLevelFilter(m_level));
  stream->push(CFLogStampFilter(m_streamName));
  stream->push(fileDescr);
  
  m_destinations[FILE] = stream;
 }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogLevelFilter & CFLogStream::getLevelFilter(CFLogDestination dest) const
{
 return *m_destinations.find(dest)->second->component<CFLogLevelFilter>(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CFLogStampFilter & CFLogStream::getStampFilter(CFLogDestination dest) const
{
 return *m_destinations.find(dest)->second->component<CFLogStampFilter>(1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CFLogStream::isFileOpen() const
{
 return m_destinations.find(FILE)->second != NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::addStringForwarder(CFLogStringForwarder * forwarder)
{
 std::list<CFLogStringForwarder *>::iterator begin = m_stringForwarders.begin();
 std::list<CFLogStringForwarder *>::iterator end = m_stringForwarders.end();
 
 if(forwarder != NULL && std::find(begin, end, forwarder) == end)
  m_stringForwarders.push_back(forwarder);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CFLogStream::removeStringForwarder(CFLogStringForwarder * forwarder)
{
 m_stringForwarders.remove(forwarder);
}