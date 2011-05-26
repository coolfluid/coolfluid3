// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_SELECT_BY_SIZE_MAX_CASE 20

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "Common/BoostFilesystem.hpp"

#include <iostream>
#include <sstream>
#include <cstdio>

#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"

using namespace boost;

using namespace CF;
using namespace CF::Common;

Logger::Logger()
{
  // streams initialization
  m_streams[ERROR]   = new LogStream("Error",   ERROR);
  m_streams[WARNING] = new LogStream("Warning", WARNING);
  m_streams[INFO]    = new LogStream("Info",    INFO);
  m_streams[DEBUG]   = new LogStream("Debug",   DEBUG);

  m_streams[ERROR]->setFilterRankZero(false);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Logger::~Logger()
{
  std::map<LogLevel, LogStream *>::iterator it;

  for(it = m_streams.begin() ; it != m_streams.end() ; it++)
    delete it->second;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Logger & Logger::instance()
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
  return *(m_streams[WARNING]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::Debug(const CodeLocation & place)
{
  return *(m_streams[DEBUG]) << place;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogStream & Logger::getStream(LogLevel type)
{
  return *(m_streams[type]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Logger::openFiles()
{
  if(mpi::PE::instance().is_active())
  {
    std::ostringstream logFile;

    iostreams::file_descriptor_sink fdLogFile;

    int rank = mpi::PE::instance().rank();

    filesystem::remove(logFile.str());

    logFile << "output-p" << rank << ".log";

    fdLogFile = iostreams::file_descriptor_sink(logFile.str());

    // setFiles
    m_streams[INFO]->setFile(fdLogFile);
    m_streams[ERROR]->setFile(fdLogFile);
    m_streams[WARNING]->setFile(fdLogFile);
    m_streams[DEBUG]->setFile(fdLogFile);
  }
}

void Logger::set_log_level(const Uint log_level)
{
  std::map<LogLevel, LogStream *>::iterator it;

  for(it = m_streams.begin() ; it != m_streams.end() ; it++)
  {
    it->second->set_log_level(log_level);
  }
}


