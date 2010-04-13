#include <boost/tokenizer.hpp>

#include "Common/CPath.hh"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

InvalidPath::InvalidPath( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "InvalidPath")
{}

////////////////////////////////////////////////////////////////////////////////

CPath::CPath () :
  m_path ()
{
}

CPath::CPath ( const CPath& path )
{
  operator=(path);
}

CPath::CPath ( const std::string& s ):
  m_path ( s )
{
  if ( ! is_valid_path(s) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +s+ "'");
}

CPath::CPath ( const char* c ):
  m_path ( c )
{
  if ( ! is_valid_path(m_path) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +m_path+ "'");
}

CPath& CPath::operator/= (const CPath& rhs)
{
  if ( ! rhs.m_path.empty() )
  {
    m_path += separator();
    m_path += rhs.m_path;
  }
  return *this;
}

CPath& CPath::operator/= (const std::string& s)
{
  if ( ! is_valid_path(s) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +s+ "'");

  if ( ! s.empty() )
  {
    m_path += separator();
    m_path += s;
  }
  return *this;
}

CPath  CPath::operator/  (const CPath& p) const
{
  if ( p.m_path.empty() )
    return *this;
  else
    return CPath ( m_path + separator() + p.m_path );
}

CPath  CPath::operator/  (const std::string& p) const
{
  return *this / CPath ( p );
}

CPath& CPath::operator=  (const CPath& p)
{
  m_path = p.m_path;
  return *this;
}

CPath& CPath::operator=  (const std::string& s)
{
  if ( ! is_valid_path(s) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +s+ "'");

  m_path = s;
  return *this;
}

bool CPath::is_valid_element ( const std::string& str )
{
  /// @todo implement validity check of the string as a path element
  return true;
}

bool CPath::is_valid_path ( const std::string& str )
{
  /// @todo implement validity check of the string as a path
  return true;
}

//CPath CPath::base_path () const
//{
//  /// @todo implement base_path
//  return m_path;
//}

const std::string& CPath::separator ()
{
  static std::string sep ( "/" );
  return sep;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
