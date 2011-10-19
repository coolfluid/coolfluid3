// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp> // for map_list_of
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "Common/BoostFilesystem.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/URI.hpp"
#include "Common/Log.hpp"
#include "Common/StringConversion.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

InvalidURI::InvalidURI( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "InvalidURI")
{}

////////////////////////////////////////////////////////////////////////////////

URI::Scheme::Convert& URI::Scheme::Convert::instance()
{
  static URI::Scheme::Convert instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

URI::Scheme::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( URI::Scheme::INVALID, "Invalid" )
      ( URI::Scheme::HTTP,    "http"    )
      ( URI::Scheme::HTTPS,   "https"   )
      ( URI::Scheme::CPATH,   "cpath"   )
      ( URI::Scheme::FILE,    "file"    );

  all_rev = boost::assign::map_list_of
      ("Invalid",  URI::Scheme::INVALID )
      ("http",     URI::Scheme::HTTP    )
      ("https",    URI::Scheme::HTTPS   )
      ("cpath",    URI::Scheme::CPATH   )
      ("file",     URI::Scheme::FILE    );
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const URI::Scheme::Type& in )
{
  os << URI::Scheme::Convert::instance().to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, URI::Scheme::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = URI::Scheme::Convert::instance().to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

URI::URI () :
  m_path (),
  m_scheme(URI::Scheme::CPATH)
{
}

URI::URI ( const URI& path )
{
  operator=(path);
}

URI::URI ( const std::string& s ):
  m_path ( s ),
  m_scheme(URI::Scheme::CPATH)
{
  split_path(s, m_scheme, m_path);
}

URI::URI ( const char* c ):
  m_path ( c ),
  m_scheme(URI::Scheme::CPATH)
{
  std::string s (c);
  split_path(s, m_scheme, m_path);
}

URI::URI ( const std::string& s, URI::Scheme::Type p ):
  m_path ( s ),
  m_scheme( p )
{
  /// @todo check path
  // throw NotImplemented(FromHere(), "Implement this");
}

bool URI::operator== (const URI& right) const
{
  return m_scheme == right.m_scheme && m_path == right.m_path;
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
  m_scheme = p.m_scheme;
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
  if (find_last(rpath,separator()).begin() == rpath.end())
    return URI::Scheme::Convert::instance().to_str(m_scheme)+":./";
  else
  {
    rpath.erase ( find_last(rpath,separator()).begin(), rpath.end() );
    return rpath;
  }
}

std::string URI::name () const
{
  using namespace boost::algorithm;
  std::string name = string();
  if (find_last(name,separator()).begin() == name.end())
    name.erase ( name.begin(), find_last(name,":").begin()+1 );
  else
    name.erase ( name.begin(), find_last(name,separator()).begin()+1 );
  return name;
}

const std::string& URI::separator ()
{
  static std::string sep ( "/" );
  return sep;
}

void URI::scheme( URI::Scheme::Type sch )
{
  m_scheme = sch;
}

void URI::path( const std::string& path )
{
  m_path = path;
}

URI::Scheme::Type URI::scheme() const
{
  return m_scheme;
}

std::string URI::path() const
{
  return m_path;
}

std::string URI::string() const
{
  // if the path is not empty, we prepend the protocol
  if(!m_path.empty())
    return URI::Scheme::Convert::instance().to_str(m_scheme) + ':' + m_path;

  return m_path;
}

void URI::split_path(const std::string & path, URI::Scheme::Type & protocol,
                     std::string & real_path)
{
  // by default the protocol is CPATH
  protocol = URI::Scheme::CPATH;
  real_path = path;

  size_t colon_pos = path.find_first_of(':');

  // if the colon has been found
  if(colon_pos != std::string::npos)
  {
    // extract the procotol
    std::string protocol_str = path.substr(0, colon_pos);

    // extract the path
    real_path = real_path.substr(colon_pos + 1, path.length() - colon_pos - 1);

    // check that the protocol is valid
    protocol = URI::Scheme::Convert::instance().to_enum(protocol_str);

    if(protocol == URI::Scheme::INVALID)
      throw ProtocolError(FromHere(), "\'" + protocol_str + "\' is not a supported protocol");
  }

}

std::string URI::extension() const
{
  const boost::filesystem::path p(path());
  return p.extension();
}

std::string URI::base_name() const
{
  const boost::filesystem::path p(path());
  return boost::filesystem::basename(p);
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////
