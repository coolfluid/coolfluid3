// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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

URI::URI () :
  m_path (),
  m_protocol(URIProtocol::INVALID)
{
}

URI::URI ( const URI& path )
{
  operator=(path);
}

URI::URI ( const std::string& s ):
  m_path ( s ),
  m_protocol(URIProtocol::INVALID)
{
  size_t colon_pos = m_path.find_first_of(':');

  if(colon_pos != std::string::npos)
  {
    std::string protocol_str = m_path.substr(0, colon_pos);

    m_path = m_path.substr(colon_pos + 1, m_path.length() - colon_pos - 1);

    m_protocol = URIProtocol::Convert::to_enum(protocol_str);

    if(m_protocol == URIProtocol::INVALID)
      throw ProtocolError(FromHere(), "\'" + protocol_str + "\' is not a supported protocol");
  }
}

URI::URI ( const char* c ):
  m_path ( c ),
  m_protocol(URIProtocol::INVALID)
{
  size_t colon_pos = m_path.find_first_of(':');

  if(colon_pos != std::string::npos)
  {
    std::string protocol_str = m_path.substr(0, colon_pos);

    m_path = m_path.substr(colon_pos + 1, m_path.length() - colon_pos - 1);

    m_protocol = URIProtocol::Convert::to_enum(protocol_str);

    if(m_protocol == URIProtocol::INVALID)
      throw ProtocolError(FromHere(), "\'" + protocol_str + "\' is not a supported protocol");
  }

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

URIProtocol::Type URI::protocol() const
{
  return m_protocol;
//  size_t colon_pos = m_path.find_first_of(':');

//  if(colon_pos != std::string::npos)
//    return m_path.substr(0, colon_pos);

//  return std::string();
}

std::string URI::string_without_protocol() const
{
  return m_path;
}

std::string URI::string() const
{
  if(m_protocol != URIProtocol::INVALID)
    return URIProtocol::Convert::to_str(m_protocol) + ':' + m_path;

  return m_path;
}

void URI::split_path(const std::string & path, URIProtocol::Type & protocol,
                     std::string & real_path)
{

}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////
