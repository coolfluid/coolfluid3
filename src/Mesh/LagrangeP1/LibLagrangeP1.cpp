// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/LagrangeP1/LibLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

CF::Common::RegistLibrary<LibLagrangeP1> LibLagrangeP1;

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP1::initiate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP1::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
