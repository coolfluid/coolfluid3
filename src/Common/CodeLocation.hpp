#ifndef CF_Common_CodeLocation_hpp
#define CF_Common_CodeLocation_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class stores the information about a location in the source code
/// @author Tiago Quintino
class Common_API CodeLocation
{
public:

  /// constructor of the code location
  explicit CodeLocation (const char * file, int line, const char * function);

  /// @returns a string where the location is
  std::string str () const;

  std::string short_str() const;

private:
  /// from which file the exception was thrown
  std::string m_file;
  /// from which function the exception was thrown
  /// @note will be empty if the compiler does not support it
  std::string m_function;
  /// from which line the exception was thrown
  int      m_line;

}; // end of class CodeLocation

////////////////////////////////////////////////////////////////////////////////

#define FromHere() CF::Common::CodeLocation( __FILE__ , __LINE__ , __FUNCTION__ )

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CodeLocation_hpp
