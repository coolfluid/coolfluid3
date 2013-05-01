// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "math/MathExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

////////////////////////////////////////////////////////////////////////////////

OutOfBounds::OutOfBounds (const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "OutOfBounds")
{}

OutOfBounds::~OutOfBounds() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ZeroDeterminant::ZeroDeterminant ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "ZeroDeterminant")
{}

ZeroDeterminant::~ZeroDeterminant() throw()
{}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

