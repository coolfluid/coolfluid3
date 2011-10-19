// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/LagrangeP2B/LibLagrangeP2B.hpp"

namespace cf3 {
namespace Mesh {
namespace LagrangeP2B {

cf3::common::RegistLibrary<LibLagrangeP2B> LibLagrangeP2B;

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP2B::initiate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP2B::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2B
} // Mesh
} // cf3
