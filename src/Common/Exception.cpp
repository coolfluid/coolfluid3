// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/Log.hpp"
#include "Common/Exception.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

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
    CFerror << m_what << CFendl;
  }

  if ( ExceptionManager::instance().ExceptionDumps )
  {
    std::string backtrace = OSystem::instance().layer()->back_trace();
    CFerror << "\n\n";
    CFerror << "+++ Exception backtrace on rank " << mpi::PE::instance().rank() << " ++++\n";
    CFerror << backtrace << "\n";
    CFerror << "++++++++++++++++++++++++++++++++++++++" << CFendl;
  }

  if ( ExceptionManager::instance().ExceptionAborts )
  {
    CFerror << CFendl << CFendl;
    CFerror << "+++ Exception aborting on rank " << mpi::PE::instance().rank() << " ... " << CFendl;
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
  desc += "\n\n";
  desc += "+++ Exception thrown on rank "+ to_str(mpi::PE::instance().rank()) + " ++++++++\n";
  desc += "From : \'";
  desc += m_where.str();
  desc += "\'\n";
  desc += "Type : \'";
  desc += m_class_name;
  desc += "\'\n";
  desc += "Message :\n";
  desc += m_msg;
  desc += "\n+++++++++++++++++++++++++++++++++++++\n";
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, CF::Common::Exception& exc)
{
  output << exc.str();
  return output;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, CF::Common::Exception* exc)
{
  return output << *exc;
}

////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
