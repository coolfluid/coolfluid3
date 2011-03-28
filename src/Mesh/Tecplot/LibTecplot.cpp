// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

void LibTecplot::initiate()
{
  cf_assert( !m_is_initiated );
  m_is_initiated = true;
}

void LibTecplot::terminate()
{
  m_is_initiated = false;
}

////////////////////////////////////////////////////////////////////////////////

} // Tecplot
} // Mesh
} // CF
