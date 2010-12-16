// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/URI.hpp"
#include "Common/Log.hpp"
#include "Common/String/Conversion.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

InvalidURI::InvalidURI( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "InvalidURI")
{}

////////////////////////////////////////////////////////////////////////////////

URI::Protocol::Convert::FwdMap_t URI::Protocol::Convert::all_fwd = boost::assign::map_list_of
    ( URI::Protocol::INVALID, "Invalid" )
    ( URI::Protocol::HTTP,    "http"    )
    ( URI::Protocol::HTTPS,   "https"   )
    ( URI::Protocol::CPATH,   "cpath"   )
    ( URI::Protocol::FILE,    "file"    );

URI::Protocol::Convert::BwdMap_t URI::Protocol::Convert::all_rev = boost::assign::map_list_of
    ("Invalid",  URI::Protocol::INVALID )
    ("http",     URI::Protocol::HTTP    )
    ("https",    URI::Protocol::HTTPS   )
    ("cpath",    URI::Protocol::CPATH   )
    ("file",     URI::Protocol::FILE    );

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const URI::Protocol::Type& in )
{
  os << URI::Protocol::Convert::to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, URI::Protocol::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = URI::Protocol::Convert::to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

URI::URI () :
  m_path (),
  m_protocol(URI::Protocol::INVALID)
{
}

URI::URI ( const URI& path )
{
  operator=(path);
}

URI::URI ( const std::string& s ):
  m_path ( s ),
  m_protocol(URI::Protocol::INVALID)
{
  split_path(s, m_protocol, m_path);
}

URI::URI ( const char* c ):
  m_path ( c ),
  m_protocol(URI::Protocol::INVALID)
{
  std::string s (c);
  split_path(s, m_protocol, m_path);
}

URI::URI ( const std::string& s, URI::Protocol::Type p ):
  m_path ( s ),
  m_protocol( p )
{
   throw NotImplemented(FromHere(), "Implement this");
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
  m_protocol = p.m_protocol;
  return *this;
}

bool URI::is_complete () const
{
  return !( boost::algorithm::starts_with( m_path, "." )  ||
            boost::algorithm::contains( m_path, "./" )    ||
            boost::algorithm::contains( m_path, "/." )    );
}

bool URI::is_valid_element ( const std::string& str )
{
  return boost::algorithm::all(str, boost::algorithm::is_alnum() ||
                                    boost::algorithm::is_any_of(".-_<,>[]()"))
         && ( str.size() )
         && ( str[0] != '.' ); // cannot start with "."
}

bool URI::is_relative () const
{
  return ! is_absolute();
}

bool URI::is_absolute () const
{
  return boost::regex_match(m_path,boost::regex("//.+"));
}

URI URI::base_path () const
{
  using namespace boost::algorithm;
  std::string rpath = string();
  rpath.erase ( find_last(rpath,separator()).begin(), rpath.end() );
  return rpath;
}

const std::string& URI::separator ()
{
  static std::string sep ( "/" );
  return sep;
}

URI::Protocol::Type URI::protocol() const
{
  return m_protocol;
}

std::string URI::string_without_protocol() const
{
  return m_path;
}

std::string URI::string() const
{
  if(m_protocol != URI::Protocol::INVALID)
    return URI::Protocol::Convert::to_str(m_protocol) + ':' + m_path;

  return m_path;
}

void URI::split_path(const std::string & path, URI::Protocol::Type & protocol,
                     std::string & real_path)
{

  protocol = URI::Protocol::INVALID;
  real_path = path;

  size_t colon_pos = path.find_first_of(':');

  if(colon_pos != std::string::npos)
  {
    std::string protocol_str = path.substr(0, colon_pos);

    real_path = real_path.substr(colon_pos + 1, path.length() - colon_pos - 1);

    protocol = URI::Protocol::Convert::to_enum(protocol_str);

    if(protocol == URI::Protocol::INVALID)
      throw ProtocolError(FromHere(), "\'" + protocol_str + "\' is not a supported protocol");
  }

}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
