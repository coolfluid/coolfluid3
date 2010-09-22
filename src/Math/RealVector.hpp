// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RealVector_hpp
#define CF_RealVector_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/LibMath.hpp"
#include "Math/VectorT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_CXX_EXPLICIT_TEMPLATES
  // explicit template instantiation
  Math_TEMPLATE template class Math_API Math::VectorT<Real>;
  Math_TEMPLATE template class Math_API Math::VectorSliceT<Real>;
#endif

/// RealVector is a VectorT templatized with a Real
typedef Math::VectorT<Real> RealVector;

/// RealSliceVector is a VectorSliceT templatized with a Real
typedef Math::VectorSliceT<Real> RealSliceVector;

////////////////////////////////////////////////////////////////////////////////

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RealVector_hpp
