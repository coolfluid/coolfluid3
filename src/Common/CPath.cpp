// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/CPath.hpp"

#include "Common/String/Conversion.hpp"

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
  if ( ! is_valid(s) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +s+ "'");
}

CPath::CPath ( const char* c ):
  m_path ( c )
{
  if ( ! is_valid(m_path) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +m_path+ "'");
}
  
CPath::CPath ( const URI& uri ) 
{
  if (uri.is_relative())
  {
    m_path = uri.string();
  }
  else
  {
    boost::regex e("cpath://.+");
    if (!boost::regex_match(uri.string(),e))
    {
      throw InvalidPath (FromHere(),"The URI \""+uri.string()+"\" doesn't refer to a component path");
    }
    m_path = uri.string();
    m_path.erase(m_path.begin(),m_path.begin()+6);
  }

  if ( ! is_valid(m_path) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +m_path+ "'");
}

CPath& CPath::operator/= (const CPath& rhs)
{
  if ( !m_path.empty() && !rhs.m_path.empty() ) m_path += separator();
  m_path += rhs.m_path;
  return *this;
}

CPath& CPath::operator/= (const std::string& s)
{
  if ( ! is_valid(s) )
    throw InvalidPath (FromHere(),"Trying to construct path with string '" +s+ "'");

  if ( !m_path.empty() && !s.empty() ) m_path += separator();
  m_path += s;

  return *this;
}

CPath  CPath::operator/  (const CPath& p) const
{
  return ( !m_path.empty() && !p.m_path.empty() ) ?
      CPath ( m_path + separator() + p.m_path ) : // both not empty
      CPath ( m_path + p.m_path );                // one is empty
}

CPath& CPath::operator=  (const CPath& p)
{
  m_path = p.m_path;
  return *this;
}

bool CPath::is_valid_element ( const std::string& str )
{
  return boost::algorithm::all(str, boost::algorithm::is_alnum() ||
                                    boost::algorithm::is_any_of(".-_<,>[]()"))
         && ( str.size() )
         && ( str[0] != '.' ); // cannot start with "."
}

bool CPath::is_valid ( const std::string& str )
{
  return boost::algorithm::all(str,
                               boost::algorithm::is_alnum() ||
                               boost::algorithm::is_any_of("./-_<,>[]()"));
}

bool CPath::is_complete () const
{
  return !( boost::algorithm::starts_with( m_path, "." )  ||
            boost::algorithm::contains( m_path, "./" )    ||
            boost::algorithm::contains( m_path, "/." )    );
}

bool CPath::is_relative () const
{
  return ! is_absolute();
}

bool CPath::is_absolute () const
{
  return boost::algorithm::starts_with( m_path, "//" );
}

CPath CPath::base_path () const
{
  using namespace boost::algorithm;
  std::string rpath = m_path;
  rpath.erase ( find_last(rpath,separator()).begin(), rpath.end() );
  return rpath;
}

const std::string& CPath::separator ()
{
  static std::string sep ( "/" );
  return sep;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
