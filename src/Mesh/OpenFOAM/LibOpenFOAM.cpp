// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CoreEnv.hpp"

#include "Mesh/OpenFOAM/LibOpenFOAM.hpp"

namespace CF {
namespace Mesh {
namespace OpenFOAM {

  CF::Common::ForceLibRegist<LibOpenFOAM> libOpenFOAM;

////////////////////////////////////////////////////////////////////////////////

void LibOpenFOAM::initiate()
{
}

void LibOpenFOAM::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // OpenFOAM
} // Mesh
} // CF
