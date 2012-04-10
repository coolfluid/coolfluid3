// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "common/Builder.hpp"
#include "EmptyLSSVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LSS::EmptyLSSVector, LSS::Vector, LSS::LibLSS > EmptyLSSVector_Builder;

} // namespace LSS
} // namespace math
} // namespace cf3
