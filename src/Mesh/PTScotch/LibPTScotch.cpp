// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/PTScotch/LibPTScotch.hpp"

namespace CF {
namespace Mesh {
namespace PTScotch {

CF::Common::RegistLibrary<LibPTScotch> libPTScotch;

////////////////////////////////////////////////////////////////////////////////

void LibPTScotch::initiate_impl()
{
}

void LibPTScotch::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // PTScotch
} // Mesh
} // CF
