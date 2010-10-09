// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Solver/ResidualDistribution.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ObjectProvider < ResidualDistribution, CDiscretization, LibSolver, NB_ARGS_1 >
ResidualDistribution_Provider ( ResidualDistribution::type_name() );

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::ResidualDistribution ( const CName& name  ) :
  CDiscretization ( name )
{
  BUILD_COMPONENT;
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
