// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "common/OptionT.hpp"

#include "physics/Variables.hpp"

namespace cf3 {
namespace physics {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

Variables::Variables( const std::string& name ) :
  Component(name)
{
}

////////////////////////////////////////////////////////////////////////////////

Variables::~Variables()
{
}

////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3
