// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/LagrangeP3/LibLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP3 {

CF::Common::RegistLibrary<LibLagrangeP3> LibLagrangeP3;

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP3::initiate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP3::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // Mesh
} // CF
