// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_MathExceptions_hpp
#define cf3_Math_MathExceptions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Exception.hpp"

#include "math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

////////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when a certain value is not found in a storage or container.
/// @author Willem Deconinck
struct Math_API OutOfBounds : public common::Exception {

  /// Constructor
  OutOfBounds (const common::CodeLocation& where, const std::string& what);
  
  virtual ~OutOfBounds() throw();

}; // end OutOfBounds

////////////////////////////////////////////////////////////////////////////////

/// @brief Exception thrown when a Zero determinant matrix is found.
/// @author Willem Deconinck
struct Math_API ZeroDeterminant  : public common::Exception {

  /// Constructor
  ZeroDeterminant (const common::CodeLocation& where, const std::string& what);
  
  virtual ~ZeroDeterminant() throw();

}; // end ZeroDeterminant

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

#endif // cf3_Math_MathExceptions_hpp

