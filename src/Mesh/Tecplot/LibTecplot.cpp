// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/Tecplot/LibTecplot.hpp"

namespace CF {
namespace Mesh {
namespace Tecplot {

CF::Common::RegistLibrary<LibTecplot> libTecplot;

////////////////////////////////////////////////////////////////////////////////

void LibTecplot::initiate_impl()
{
}

void LibTecplot::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Tecplot
} // Mesh
} // CF
