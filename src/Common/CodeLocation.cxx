#include <cstdio>

#include "Common/CodeLocation.hh"

//////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////

CodeLocation::CodeLocation(const char * file, int line, const char * function)
 : m_file(file), m_function(function), m_line (line)
{
}

//////////////////////////////////////////////////////////////////////

std::string CodeLocation::str () const
{
  char line [50];
  sprintf (line, "%d", m_line);
  std::string place (m_file);
  place += ":";
  place += line;
  if (!m_function.empty()) // skip if compiler does not set function
  {
    place += ":";
    place += m_function;
  }
  return place;
}

//////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF
