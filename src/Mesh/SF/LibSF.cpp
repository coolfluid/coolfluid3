// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

  CF::Common::ForceLibRegist<LibSF> libSF;

////////////////////////////////////////////////////////////////////////////////

void LibSF::initiate()
{
}

void LibSF::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
