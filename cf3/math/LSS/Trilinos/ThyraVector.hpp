// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_ThyraVector_hpp
#define cf3_Math_LSS_ThyraVector_hpp

#include "Teuchos_RCP.hpp"
#include "Thyra_MultiVectorBase.hpp"

#include "common/CF.hpp"

namespace cf3 {
namespace math {
namespace LSS {

/// Abstract class for all Trilinos matrix operators
class ThyraVector
{
public:
  /// Const access to the matrix
  virtual Teuchos::RCP<const Thyra::VectorBase<Real> > thyra_vector() const = 0;
  
  /// Writable access to the matrix
  virtual Teuchos::RCP<Thyra::VectorBase<Real> > thyra_vector() = 0;
};

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_ThyraVector_hpp
