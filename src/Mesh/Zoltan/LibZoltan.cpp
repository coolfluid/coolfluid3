// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/Zoltan/LibZoltan.hpp"

namespace CF {
namespace Mesh {
namespace Zoltan {

CF::Common::RegistLibrary<LibZoltan> libZoltan;

////////////////////////////////////////////////////////////////////////////////

void LibZoltan::initiate_impl()
{
}

void LibZoltan::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Zoltan
} // Mesh
} // CF
