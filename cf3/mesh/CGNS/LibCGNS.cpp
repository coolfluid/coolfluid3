// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "mesh/CGNS/LibCGNS.hpp"

namespace cf3 {
namespace mesh {
namespace CGNS {

cf3::common::RegistLibrary<LibCGNS> libCGNS;

////////////////////////////////////////////////////////////////////////////////

void LibCGNS::initiate_impl()
{
}

void LibCGNS::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // CGAL
} // mesh
} // cf3
