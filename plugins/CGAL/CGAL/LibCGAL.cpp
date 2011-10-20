// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "CGAL/LibCGAL.hpp"

namespace cf3 {
namespace Mesh {
namespace CGAL {

cf3::common::RegistLibrary<LibCGAL> libCGAL;

////////////////////////////////////////////////////////////////////////////////

void LibCGAL::initiate_impl()
{
}

void LibCGAL::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // CGAL
} // Mesh
} // CF
