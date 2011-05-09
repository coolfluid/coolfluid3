// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_MathExceptions_hpp
#define CF_Math_MathExceptions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

#include "Math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when a certain value is not found in a storage or container.
/// @author Willem Deconinck
struct Math_API OutOfBounds : public Common::Exception {

  /// Constructor
  OutOfBounds (const Common::CodeLocation& where, const std::string& what);
  
  virtual ~OutOfBounds() throw();

}; // end OutOfBounds

////////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when a Zero determinant matrix is found.
/// @author Willem Deconinck
struct Math_API ZeroDeterminant  : public Common::Exception {

  /// Constructor
  ZeroDeterminant (const Common::CodeLocation& where, const std::string& what);
  
  virtual ~ZeroDeterminant() throw();

}; // end ZeroDeterminant

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

#endif // CF_Math_MathExceptions_hpp

