// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Math/MathExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Math {

////////////////////////////////////////////////////////////////////////////////

OutOfBounds::OutOfBounds (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "OutOfBounds")
{}

////////////////////////////////////////////////////////////////////////////////

ZeroDeterminant::ZeroDeterminant ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ZeroDeterminant")
{}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math
} // namespace CF

