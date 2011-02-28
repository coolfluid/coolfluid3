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
#include "Common/EnumT.hpp"

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

    class Common_API Scheme
    {
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

    }; // Protocol

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
    URI ( const std::string& s, URI::Scheme::Type p );

    // operators

    /// comparison operator
    bool operator== (const URI& right) const;

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

    /// @return the full URI as a string
    std::string string() const;

    /// @return the base path
    URI base_path() const;

    /// @return the the name of the object, without the path
    std::string name() const;

    /// check that the passed string is a valid path element
    static bool is_valid_element ( const std::string& str);

    /// separator for path tokens
    static const std::string& separator ();

    /// Gives the protocol (if any).
    /// @return Returns the protocol. May return @c URI::Protocol::INVALID if no
    /// protocol has been specified.
    Scheme::Type scheme() const;

    /// Gives the URI path, which is the URI without the scheme (protocol)
    /// @return Returns the URI path
    std::string path() const;

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

  private:

    /// path string
    std::string m_path;
    /// Current URI protocol
    Scheme::Type m_scheme;

  }; // URI

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_URI_hpp
