// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

CF::Common::RegistLibrary<LibSF> libSF;

////////////////////////////////////////////////////////////////////////////////

void LibSF::initiate_impl()
{
}

void LibSF::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
