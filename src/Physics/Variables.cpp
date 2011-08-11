// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "Common/OptionT.hpp"

#include "Physics/Variables.hpp"

namespace CF {
namespace Physics {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Variables::Variables( const std::string& name ) :
  Component(name)
{
}

Variables::~Variables()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
