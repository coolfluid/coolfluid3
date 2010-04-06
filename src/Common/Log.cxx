#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp> 

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>

#include "Log.hh"
#include "StringOps.hh"
#include "PE.hh"

using namespace CF;
using namespace CF::Common;
using namespace boost;

Logger::Logger()
{

 // streams initialization
 this->m_streams[INFO_STREAM] = new LogStream("INFO"); 
 this->m_streams[ERROR_STREAM] = new LogStream("ERROR");
 this->m_streams[WARN_STREAM] = new LogStream("WARNING");
 this->m_streams[DEBUG_STREAM] = new LogStream("DEBUG"); 
 this->m_streams[TRACE_STREAM] = new LogStream("TRACE");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Logger::~Logger()
{
 std::map<StreamType, LogStream *>::iterator it;
 
 for(it = this->m_streams.begin() ; it != this->m_streams.end() ; it++)
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

LogStream & Logger::getInfo (const CodeLocation & place)
{
 return *(this->m_streams[INFO_STREAM]) << place;
}
   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getError(const CodeLocation & place)
{
 return *(this->m_streams[ERROR_STREAM]) << place;
}
   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getWarn(const CodeLocation & place)
{
 return *(this->m_streams[WARN_STREAM]) << place;
}
   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getDebug(const CodeLocation & place)
{
 return *(this->m_streams[DEBUG_STREAM]) << place;
}
   
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getTrace(const CodeLocation & place)
{
 return *(this->m_streams[TRACE_STREAM]) << place;
}

void Logger::openFiles()
{
 if(PE::get_instance().is_init())
 {
  std::ostringstream logFile;
  std::ostringstream traceFile;
  iostreams::filtering_ostream * logFileStream;
  iostreams::filtering_ostream * traceFileStream;
  iostreams::file_descriptor_sink fdLogFile;
  iostreams::file_descriptor_sink fdTraceFile;
  int rank = PE::get_instance().get_rank();
 
  logFile << "output-p" << rank << ".log";
  traceFile << "output-p" << rank << ".trace";
 
  filesystem::remove(logFile.str());
  filesystem::remove(traceFile.str());
 
  fdLogFile = iostreams::file_descriptor_sink(logFile.str());
  fdTraceFile = iostreams::file_descriptor_sink(traceFile.str());
 
 // setFiles
  this->m_streams[INFO_STREAM]->setFile(fdLogFile); 
  this->m_streams[ERROR_STREAM]->setFile(fdLogFile);
  this->m_streams[WARN_STREAM]->setFile(fdLogFile);
  this->m_streams[DEBUG_STREAM]->setFile(fdLogFile); 
  this->m_streams[TRACE_STREAM]->setFile(fdTraceFile);
 }
}