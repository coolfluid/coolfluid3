#ifndef CF_Math_MathExceptions_hpp
#define CF_Math_MathExceptions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a certain value is not found in a storage or container.
/// @author Willem Deconinck
struct OutOfBounds : public Common::Exception {

  /// Constructor
  OutOfBounds (const Common::CodeLocation& where, const std::string& what);

}; // end OutOfBounds

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a Zero determinant matrix is found.
/// @author Willem Deconinck
struct ZeroDeterminant  : public Common::Exception {

  /// Constructor
  ZeroDeterminant (const Common::CodeLocation& where, const std::string& what);

}; // end ZeroDeterminant

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

#endif // CF_Math_MathExceptions_hpp

