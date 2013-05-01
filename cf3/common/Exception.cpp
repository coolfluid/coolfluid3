// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/Log.hpp"
#include "common/Exception.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/PE/Comm.hpp"
#include "common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace common {

////////////////////////////////////////////////////////////////////////////////

ExceptionManager::ExceptionManager() :
  ExceptionOutputs ( true ),
  ExceptionDumps   ( true ),
  ExceptionAborts  ( false )
  {
    std::set_terminate(std::abort);
  }

////////////////////////////////////////////////////////////////////////////////

ExceptionManager& ExceptionManager::instance()
{
  static ExceptionManager exception_manager;
  return exception_manager;
}

////////////////////////////////////////////////////////////////////////////////

Exception::Exception(CodeLocation where, std::string msg, std::string className) throw ()
  : m_where(where), m_msg(msg), m_class_name(className)
{
  m_what = full_description();
  if ( ExceptionManager::instance().ExceptionOutputs )
  {    
    CFerror << "\n\n" << m_what << CFendl;
  }

  if ( ExceptionManager::instance().ExceptionDumps )
  {
    std::string backtrace = OSystem::instance().layer()->back_trace();
    CFerror << "\n\n";
    CFerror << "+++ Exception backtrace on rank " << PE::Comm::instance().rank() << " ++++++++++++++++++++++++++++++++++++++++++\n";
    CFerror << backtrace << "\n";
    CFerror << "++++++++++++++++++++++++++++++++++++++" << CFendl;
  }

  if ( ExceptionManager::instance().ExceptionAborts )
  {
    CFerror << CFendl << CFendl;
    CFerror << "+++ Exception aborting on rank " << PE::Comm::instance().rank() << " ... " << CFendl;
    abort();
  }
}

////////////////////////////////////////////////////////////////////////////////

Exception::~Exception() throw ()
{
}

////////////////////////////////////////////////////////////////////////////////

void Exception::append(const std::string& add) throw ()
{
  m_what += add;
}

////////////////////////////////////////////////////////////////////////////////

const std::string& Exception::str () const throw ()
{
  return m_what;
}

////////////////////////////////////////////////////////////////////////////////

const char* Exception::what() const throw()
{
  return str().c_str();
}

////////////////////////////////////////////////////////////////////////////////

std::string Exception::full_description () const throw ()
{
  std::string desc;
  desc += "+++ Exception thrown on rank "+ to_str(PE::Comm::instance().rank()) + " ++++++++++++++++++++++++++++++++++++++++++++++\n";
  desc += "From : \'";
  desc += m_where.str();
  desc += "\'\n";
  desc += "Type : \'";
  desc += m_class_name;
  desc += "\'\n";
  desc += "Message :\n";
  desc += m_msg;
  desc += "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, cf3::common::Exception& exc)
{
  output << exc.str();
  return output;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, cf3::common::Exception* exc)
{
  return output << *exc;
}

////////////////////////////////////////////////////////////////////////////////

  } // common
} // cf3

////////////////////////////////////////////////////////////////////////////////
