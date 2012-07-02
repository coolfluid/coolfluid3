// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file URI.hpp
/// @brief Uniform Resource Identifier (see http://en.wikipedia.org/wiki/Uniform_Resource_Identifier)
/// @note This header gets included indirectly in common/Component.hpp
///       It should be as lean as possible!

#ifndef cf3_common_URI_hpp
#define cf3_common_URI_hpp

#include <iosfwd> // forward declarations for <ios>

#include "common/CF.hpp"
#include "common/Exception.hpp"
#include "common/EnumT.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a string does not construct a valid path
struct Common_API InvalidURI: public common::Exception {

  /// Constructor
  InvalidURI( const common::CodeLocation& where, const std::string& what);

}; // InvalidPath

////////////////////////////////////////////////////////////////////////////////////////////

/// Uniform Resource Identifier (see http://en.wikipedia.org/wiki/Uniform_Resource_Identifier)
/// Used to describe a component path (cpath://), a file path (file://, or a URL (http://, https://)
/// @author Willem Deconinck
class Common_API URI {

public:

  class Common_API Scheme {
  public:

    /// Enumeration of the Shapes recognized in CF
    enum Type  { INVALID = -1,
                 HTTP    = 0,
                 HTTPS   = 1,
                 CPATH   = 2,
                 FILE    = 3
               };

    typedef EnumT< Scheme > ConverterBase;

    struct Common_API Convert : public ConverterBase
    {
      /// constructor where all the converting maps are built
      Convert();
      /// get the unique instance of the converter class
      static Convert& instance();
    };

    // std::ostream is forward declared!
    friend std::ostream& operator<< ( std::ostream& os, const URI::Scheme::Type& in );
    friend std::istream& operator>> ( std::istream& is, URI::Scheme::Type& in );

  }; // Protocol

  // static methods

  /// check that the passed string is a valid path element
  static bool is_valid_element ( const std::string& str);

  /// separator for path tokens
  static const std::string separator ();

  // constructors

  /// Empty constructor
  URI ();
  /// Copy constructor from other path object
  /// @param path object
  URI ( const URI& path );
  /// Constructor from string object
  /// @param s string with path
  URI ( const std::string& s );
  /// Constructor from const char*
  /// @param c C string with path
  URI ( const char* c );
  /// Constructor from string object and separate protocol
  /// @pre assumes that string does not have a protocol, just the path
  /// @param s string with path
  /// @param p scheme type e.g. (HTTP,CPATH,FILE)
  URI ( const std::string& s, URI::Scheme::Type p );

  // operators

  /// comparison operator
  bool operator== (const URI& right) const;
  bool operator!= (const URI& right) const;

  /// assignement operator with URI
  URI& operator=  (const URI& p);

  /// concatenation and assignement operator with URI
  URI& operator/= (const URI& rhs);
  /// concatenation and assignement operator with std::string
  URI& operator/= (const std::string& s);
  /// concatenation and assignement operator with C string
  URI& operator/= ( const char* c );

  /// concatenation operator with URI
  URI  operator/  (const URI& p) const;

  // std::ostream and std::istream are forward declared!
  friend std::ostream& operator<< ( std::ostream& os, const URI::Scheme::Type& in );
  friend std::istream& operator>> ( std::istream& is, URI::Scheme::Type& in );

  // accessors

  /// Check if path is absolute.
  /// Should start with "//"
  /// @returns true if the path is absolute
  bool is_absolute() const;

  /// Check is path is relative.
  /// Should not start with "//"
  /// @returns true if the path is a relative path
  bool is_relative() const;

  /// check this path is complete
  /// @post true if does not contain ".." or "."
  bool is_complete() const;

  /// @return if the path is empty
  bool empty() const { return m_path.empty(); }

  /// @return the full URI as a string
  std::string string() const;

  /// @return the base path
  URI base_path() const;

  /// @return the name of the object, without the path
  std::string name() const;

  /// Gives the protocol (if any).
  /// @return Returns the protocol. May return @c URI::Protocol::INVALID if no
  /// protocol has been specified.
  Scheme::Type scheme() const;

  /// Changes the protocol to the supplied scheme
  /// @post scheme() will return the supplied protocol
  void scheme( Scheme::Type );

  /// Gives the URI path, which is the URI without the scheme (protocol)
  /// @return Returns the URI path
  std::string path() const;

  /// Changes the URI path
  /// @post path() will return the supplied path
  void path( const std::string& path );

  /// Overloading of the stream operator "<<" for the output.
  /// No "\n"ine introduced.
  /// @param [in] out the out stream
  /// @param [in] path the path to output
  /// @return the out stream
  friend std::ostream& operator<< (std::ostream& out, const URI& path);

  /// Overloading of the stream operator ">>" for the input
  /// @param [in] in the in stream
  /// @param [out] path the path to read
  /// @return the in stream
  friend std::istream& operator>> (std::istream& in, URI& path);

  /// Splits a given path into the URI protocol and the real path.
  /// @param path Path to split.
  /// @param protocol Variable where to store the found protocol. The value is
  /// set to @c URI::Protocol::INVALID if no protocol found.
  /// @param real_path Variable where to store the path.
  /// @throw ProtocolError If a an unknown protocol is found.
  static void split_path(const std::string & path, URI::Scheme::Type & protocol,
                         std::string & real_path);

  /// Extension of a filename
  std::string extension() const;

  /// Filename without extension
  std::string base_name() const;

private:
  /// Cleans up the stored string, i.e. remove multiple / in sequence, ...
  void cleanup();

  /// path string
  std::string m_path;
  /// Current URI protocol
  Scheme::Type m_scheme;

}; // URI

////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<< ( std::ostream& os, const URI::Scheme::Type& scheme );
std::istream& operator>> ( std::istream& is, URI::Scheme::Type& scheme );
std::ostream& operator<< ( std::ostream& os, const URI& uri );
std::istream& operator>> ( std::istream& is, URI& uri );

} // common
} // cf3

#endif // cf3_common_URI_hpp
