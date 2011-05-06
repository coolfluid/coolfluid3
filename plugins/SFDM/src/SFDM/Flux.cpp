    // Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Mesh/GeoShape.hpp"

#include "SFDM/Flux.hpp"
#include "SFDM/ShapeFunction.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {
  
  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Flux, Component, LibSFDM> Flux_Builder;

//////////////////////////////////////////////////////////////////////////////

Flux::Flux( const std::string& name )
: Component(name)
{   
  properties()["brief"] = std::string("Compute Flux from a Solution state");
  properties()["description"] = std::string("The polynomial order \"P\" of the solution is configurable, default: P = 0");
}
  
/////////////////////////////////////////////////////////////////////////////

RealMatrix Flux::operator()(const RealMatrix& states) const
{
  /// Linear flux:  f = a * u     with a=1
  const Real a=1.;
  return a*states;
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
