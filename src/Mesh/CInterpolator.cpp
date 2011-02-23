// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

CInterpolator::CInterpolator ( const std::string& name  ) :
  Component ( name )
{

}

////////////////////////////////////////////////////////////////////////////////

CInterpolator::~CInterpolator()
{
}

//////////////////////////////////////////////////////////////////////////////

void CInterpolator::interpolate( Signal::arg_t& node  )
{
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
