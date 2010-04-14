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

    /// Empty constructor
    CPath ();
    /// Copy constructor from other path object
    CPath ( const CPath& path );
    /// Constructor from string object
    CPath ( const std::string& s );
    /// Constructor from const char*
    CPath ( const char* c );

    CPath& operator/= (const CPath& rhs);
    CPath& operator/= (const std::string& s);

    CPath  operator/  (const CPath& p) const;
    CPath  operator/  (const std::string& s) const;

    CPath& operator=  (const CPath& p);
    CPath& operator=  (const std::string& s);

    /// @return if the path is empty
    bool empty() const { return m_path.empty(); }
    /// @return the path as a string
    const std::string& string() const { return m_path; }

//    /// @return the base path
//    CPath base_path() const;

    /// check that the passed string is a valid path
    /// @post string does not contain ";,./"
    static bool is_valid_path ( const std::string& str);
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
