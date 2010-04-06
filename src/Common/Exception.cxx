#include <iostream>

#include "Common/Exception.hh"
#include "Common/OSystem.hh"
#include "Common/ProcessInfo.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

ExceptionManager::ExceptionManager() :
  ExceptionOutputs ( false ),
  ExceptionDumps   ( false ),
  ExceptionAborts  ( false ) {}

//////////////////////////////////////////////////////////////////////////////

ExceptionManager& ExceptionManager::getInstance()
{
  static ExceptionManager exception_manager;
  return exception_manager;
}

//////////////////////////////////////////////////////////////////////////////

Exception::Exception(CodeLocation where, std::string msg, std::string className) throw ()
  : m_where(where), m_msg(msg), m_class_name(className)
{
  m_what = full_description();

  if ( ExceptionManager::getInstance().ExceptionOutputs )
  {
    std::cout << "+++ Exception thrown +++++++++++++++++" << std::endl;
    std::cout << m_what << std::endl;
    std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
  }

  if ( ExceptionManager::getInstance().ExceptionDumps )
  {
    std::string backtrace = OSystem::getInstance().getProcessInfo()->getBackTrace();
    std::cout << "+++ Exception backtrace ++++++++++++++" << std::endl;
    std::cout << backtrace << std::endl;
    std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
  }

  if ( ExceptionManager::getInstance().ExceptionAborts )
  {
    std::cout << "+++ Exception aborting ... " << std::endl;
    abort();
  }
}

//////////////////////////////////////////////////////////////////////////////

Exception::~Exception() throw ()
{
}

//////////////////////////////////////////////////////////////////////////////

void Exception::append(const std::string& add) throw ()
{
  m_what += add;
}

//////////////////////////////////////////////////////////////////////////////

const std::string& Exception::str () const throw ()
{
  return m_what;
}

//////////////////////////////////////////////////////////////////////////////

const char* Exception::what() const throw()
{
  return str().c_str();
}

//////////////////////////////////////////////////////////////////////////////

std::string Exception::full_description () const throw ()
{
  std::string desc;
  desc += "+++ Exception thrown ++++++++++++++++\n";
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

//////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, CF::Common::Exception& exc)
{
  output << exc.str();
  return output;
}

//////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, CF::Common::Exception* exc)
{
  return output << *exc;
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////
