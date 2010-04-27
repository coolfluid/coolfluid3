#ifndef CF_Common_CPath_HH
#define CF_Common_CPath_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/Exception.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Exception thrown when a string does not construct a valid path
  struct Common_API InvalidPath: public Common::Exception {

    /// Constructor
    InvalidPath( const Common::CodeLocation& where, const std::string& what);

  }; // InvalidPath

  /// Base class for defining the path to a component
  /// @author Tiago Quintino
  class Common_API CPath {

  public:

    // constructors

    /// Empty constructor
    CPath ();
    /// Copy constructor from other path object
    /// @param path object
    CPath ( const CPath& path );
    /// Constructor from string object
    /// @param s string with path
    CPath ( const std::string& s );
    /// Constructor from const char*
    /// @param c C string with path
    CPath ( const char* c );

    // operators

    /// assignement operator with CPath
    CPath& operator=  (const CPath& p);

    /// concatenation and assignement operator with CPath
    CPath& operator/= (const CPath& rhs);
    /// concatenation and assignement operator with std::string
    CPath& operator/= (const std::string& s);

    /// concatenation operator with CPath
    CPath  operator/  (const CPath& p) const;

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
    std::string string() const { return m_path; }

    /// @return the base path
    CPath base_path() const;

    /// check that the passed string is a valid path
    /// @post string does not contain ";,"
    static bool is_valid ( const std::string& str);

    /// check that the passed string is a valid path element
    static bool is_valid_element ( const std::string& str);

    /// separator for path tokens
    static const std::string& separator ();

  private:

    /// path string
    std::string m_path;

  }; // CPath

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CPath_HH
