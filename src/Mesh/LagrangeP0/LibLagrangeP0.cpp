// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"

namespace cf3 {
namespace Mesh {
namespace LagrangeP0 {

cf3::common::RegistLibrary<LibLagrangeP0> LibLagrangeP0;

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP0::initiate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP0::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // cf3
