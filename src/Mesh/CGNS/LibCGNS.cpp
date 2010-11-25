// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLib.hpp"

#include "Mesh/CGNS/LibCGNS.hpp"

namespace CF {
namespace Mesh {
namespace CGNS {

CF::Common::ForceLibRegist<LibCGNS> libCGNS;

////////////////////////////////////////////////////////////////////////////////

void LibCGNS::initiate()
{
}

void LibCGNS::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // CGAL
} // Mesh
} // CF
