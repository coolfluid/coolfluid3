// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "common/BoostFilesystem.hpp"

#include <iostream>
#include <sstream>
#include <cstdio>

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Log.hpp"
#include "common/PE/Comm.hpp"
#include "common/OptionList.hpp"

using namespace boost;

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

Logger::Logger()
{
  // streams initialization
  m_streams[ERROR]   = new LogStream("Error",   ERROR);
  m_streams[WARNING] = new LogStream("Warning", WARNING);
  m_streams[INFO]    = new LogStream("Info",    INFO);
  m_streams[DEBUG]   = new LogStream("Debug",   DEBUG);

  m_streams[ERROR]->setFilterRankZero( true );

}

//////////////////////////////////////////////////////////////////////////////

Logger::~Logger()
{
  std::map<LogLevel, LogStream *>::iterator it;

  for(it = m_streams.begin() ; it != m_streams.end() ; it++)
    delete it->second;
}

//////////////////////////////////////////////////////////////////////////////

Logger & Logger::instance()
{
  static Logger log;
  return log;
}

//////////////////////////////////////////////////////////////////////////////

void Logger::initiate()
{
  bool rank0 = Core::instance().environment().options().value<bool>("only_cpu0_writes");

  CFerror.setFilterRankZero( rank0 );
  CFwarn.setFilterRankZero( rank0 );
  CFinfo.setFilterRankZero( rank0 );
  CFdebug.setFilterRankZero( rank0 );
}

//////////////////////////////////////////////////////////////////////////////

LogStream & Logger::Info (const CodeLocation & place)
{
  return *(m_streams[INFO]) << place;
}

//////////////////////////////////////////////////////////////////////////////

LogStream & Logger::Error(const CodeLocation & place)
{
  return *(m_streams[ERROR]) << place;
}

//////////////////////////////////////////////////////////////////////////////

LogStream & Logger::Warn(const CodeLocation & place)
{
  return *(m_streams[WARNING]) << place;
}

//////////////////////////////////////////////////////////////////////////////

LogStream & Logger::Debug(const CodeLocation & place)
{
  return *(m_streams[DEBUG]) << place;
}

//////////////////////////////////////////////////////////////////////////////

LogStream & Logger::getStream(LogLevel type)
{
  return *(m_streams[type]);
}

//////////////////////////////////////////////////////////////////////////////

void Logger::openFiles()
{
  if(PE::Comm::instance().is_active())
  {
    std::ostringstream logFile;

    iostreams::file_descriptor_sink fdLogFile;

    int rank = PE::Comm::instance().rank();

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

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
