// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/ResidualDistribution.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < ResidualDistribution, CDiscretization, LibSolver > ResidualDistribution_Builder;

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::ResidualDistribution ( const std::string& name  ) :
  CDiscretization ( name )
{
  tag_component(this);
  properties()["brief"] = std::string("Residual Distribution Method");
  properties()["description"] = std::string("Discretize the PDE's using the Residual Distribution Method");
}

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution()
{
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::do_whatever()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
