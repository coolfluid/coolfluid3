// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLib.hpp"

#include "Mesh/Neu/LibNeu.hpp"

namespace CF {
namespace Mesh {
namespace Neu {

CF::Common::ForceLibRegist<LibNeu> libNeu;

////////////////////////////////////////////////////////////////////////////////

void LibNeu::initiate()
{
}

void LibNeu::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF
