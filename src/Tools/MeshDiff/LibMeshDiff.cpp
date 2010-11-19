// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Core.hpp"

#include "Tools/MeshDiff/LibMeshDiff.hpp"

namespace CF {
namespace Tools {
namespace MeshDiff {

CF::Common::ForceLibRegist<LibMeshDiff> libMeshDiff;

////////////////////////////////////////////////////////////////////////////////

void LibMeshDiff::initiate()
{
}

void LibMeshDiff::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // CF
