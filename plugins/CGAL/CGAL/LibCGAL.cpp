// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "CGAL/LibCGAL.hpp"

namespace CF {
namespace Mesh {
namespace CGAL {

CF::Common::RegistLibrary<LibCGAL> libCGAL;

////////////////////////////////////////////////////////////////////////////////

void LibCGAL::initiate()
{
  cf_assert( !m_is_initiated );
  m_is_initiated = true;
}

void LibCGAL::terminate()
{
  m_is_initiated = false;
}

////////////////////////////////////////////////////////////////////////////////

} // CGAL
} // Mesh
} // CF
