#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "Common/URI.hpp"
#include "Common/Log.hpp"
#include "Common/StringOps.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

InvalidURI::InvalidURI( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "InvalidURI")
{}

////////////////////////////////////////////////////////////////////////////////

URI::URI () :
  m_path ()
{
}

URI::URI ( const URI& path )
{
  operator=(path);
}

URI::URI ( const std::string& s ):
  m_path ( s )
{
}

URI::URI ( const char* c ):
  m_path ( c )
{
}

URI& URI::operator/= (const URI& rhs)
{
  if ( !m_path.empty() && !rhs.m_path.empty() ) m_path += separator();
  m_path += rhs.m_path;
  return *this;
}

URI& URI::operator/= (const std::string& s)
{
  if ( !m_path.empty() && !s.empty() ) m_path += separator();
  m_path += s;

  return *this;
}
  
URI& URI::operator/= ( const char* c )
{
  std::string s(c);
  return operator/= (s);
}

URI  URI::operator/  (const URI& p) const
{
  return ( !m_path.empty() && !p.m_path.empty() ) ?
      URI ( m_path + separator() + p.m_path ) : // both not empty
      URI ( m_path + p.m_path );                // one is empty
}

URI& URI::operator=  (const URI& p)
{
  m_path = p.m_path;
  return *this;
}

bool URI::is_relative () const
{
  return ! is_absolute();
}

bool URI::is_absolute () const
{
  return boost::regex_match(m_path,boost::regex("[a-z]+://.+"));
}

URI URI::base_path () const
{
  using namespace boost::algorithm;
  std::string rpath = m_path;
  rpath.erase ( find_last(rpath,separator()).begin(), rpath.end() );
  return rpath;
}

const std::string& URI::separator ()
{
  static std::string sep ( "/" );
  return sep;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
