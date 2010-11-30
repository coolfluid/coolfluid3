// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_URI_hpp
#define CF_Common_URI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Exception thrown when a string does not construct a valid path
  struct Common_API InvalidURI: public Common::Exception {

    /// Constructor
    InvalidURI( const Common::CodeLocation& where, const std::string& what);

  }; // InvalidPath

  /// Base class for defining the path to a component
  /// @author Willem Deconinck
  class Common_API URI {

  public:

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

    // operators

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

    /// @return the path as a string
    std::string string() const;

    /// @return the base path
    URI base_path() const;

    /// separator for path tokens
    static const std::string& separator ();

    /// Checks whether the specified protocol is present in this URI.
    /// @param protocol The protocol to check
    /// @return Returns @c true if the protocol is present or if it is empty.
    /// Otherwise, returns @c false.
    bool is_protocol(const std::string & protocol) const;

    /// Gives the protocol (if any).
    /// @return Returns the protocol. May return an empty string if there is no
    /// protocol.
    std::string protocol() const;

    /// Gives the string value without the protocol
    /// @return Returns the string without the protocol
    std::string string_without_protocol() const;

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

  private:

    /// path string
    std::string m_path;

  }; // URI

////////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<< (std::ostream& out, const URI& path)
{
  out << path.string();
  return out;
}

////////////////////////////////////////////////////////////////////////////////

inline std::istream& operator>> (std::istream& in, URI& path)
{
  std::string path_str;
  in >> path_str;
  path = URI(path_str);
  return in;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_URI_hpp
