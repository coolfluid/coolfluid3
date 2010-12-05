// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLib.hpp"

#include "Mesh/PTScotch/LibPTScotch.hpp"

namespace CF {
namespace Mesh {
namespace PTScotch {

CF::Common::ForceLibRegist<LibPTScotch> libPTScotch;

////////////////////////////////////////////////////////////////////////////////

void LibPTScotch::initiate()
{
}

void LibPTScotch::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // PTScotch
} // Mesh
} // CF
