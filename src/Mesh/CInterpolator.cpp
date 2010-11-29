// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CInterpolator::CInterpolator ( const std::string& name  ) :
  Component ( name )
{
  tag_component(this);
}

////////////////////////////////////////////////////////////////////////////////

CInterpolator::~CInterpolator()
{
}

//////////////////////////////////////////////////////////////////////////////

void CInterpolator::interpolate( XmlNode& node  )
{
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
