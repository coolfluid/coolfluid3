#define BOOST_SELECT_BY_SIZE_MAX_CASE 20

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>

#include <stdio.h>

#include "Common/Log.hh"
#include "Common/PE.hh"

using namespace CF;
using namespace CF::Common;
using namespace boost;

Logger::Logger()
{
  // streams initialization
  m_streams[INFO] = new LogStream("INFO");
  m_streams[ERROR] = new LogStream("ERROR");
  m_streams[WARN] = new LogStream("WARNING");
  m_streams[DEBUG] = new LogStream("DEBUG");
  m_streams[TRACE] = new LogStream("TRACE");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Logger::~Logger()
{
  std::map<StreamType, LogStream *>::iterator it;
  
  for(it = m_streams.begin() ; it != m_streams.end() ; it++)
    delete it->second;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Logger & Logger::getInstance()
{
  static Logger log;
  return log;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Info (const CodeLocation & place)
{
  return *(m_streams[INFO]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Error(const CodeLocation & place)
{
  return *(m_streams[ERROR]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Warn(const CodeLocation & place)
{
  return *(m_streams[WARN]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Debug(const CodeLocation & place)
{
  return *(m_streams[DEBUG]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Trace(const CodeLocation & place)
{
  return *(m_streams[TRACE]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getStream(Logger::StreamType type)
{
  return *(m_streams[type]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Logger::openFiles()
{
  if(PE::getInstance().is_init())
  {
    std::ostringstream logFile;
    std::ostringstream traceFile;
    
    iostreams::file_descriptor_sink fdLogFile;
    iostreams::file_descriptor_sink fdTraceFile;
    
    int rank = PE::getInstance().get_rank();
    
    filesystem::remove(logFile.str());
    filesystem::remove(traceFile.str());

    logFile << "output-p" << rank << ".log";
    traceFile << "output-p" << rank << ".trace";
    
    fdLogFile = iostreams::file_descriptor_sink(logFile.str());
	fdTraceFile = iostreams::file_descriptor_sink(traceFile.str());
    
    // setFiles
    m_streams[INFO]->setFile(fdLogFile);
    m_streams[ERROR]->setFile(fdLogFile);
    m_streams[WARN]->setFile(fdLogFile);
    m_streams[DEBUG]->setFile(fdLogFile);
    m_streams[TRACE]->setFile(fdTraceFile);
  }			 
}
