// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Actions/CLoop.hpp"
#include "Actions/CForAllElementsT.hpp"
#include "Actions/CForAllNodes.hpp"

#include "Solver/ResidualDistribution.hpp"
#include "Solver/CSchemeLDA.hpp"
#include "Solver/CSchemeLDAT.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP1.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Actions;

namespace CF {
namespace Solver {

Common::ComponentBuilder < ResidualDistribution, CDiscretization, LibSolver > ResidualDistribution_Builder;

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::ResidualDistribution ( const std::string& name  ) :
  CDiscretization ( name )
{
   
  properties()["brief"] = std::string("Residual Distribution Method");
  properties()["description"] = std::string("Discretize the PDE's using the Residual Distribution Method");
}

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution()
{
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::compute_rhs()
{
  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  CLoop::Ptr apply_bc = create_component_type< CForAllNodes >("apply_bc");
  apply_bc->create_action("CF.Actions.CSetFieldValues");

  std::vector<URI> bc_regions = list_of(URI("cpath://Root/mesh/rotation/inlet"));
  apply_bc->configure_property("Regions",bc_regions);
  apply_bc->action("CF.Actions.CSetFieldValues").configure_property("Field",std::string("solution"));

    CLoop::Ptr elem_loop;

  // Static version, templates
//  elem_loop =
//      create_component_type< CForAllElementsT<CSchemeLDA> >("loop_LDA");

  elem_loop =
      create_component_type< CForAllElementsT< CSchemeLDAT< SF::Quad2DLagrangeP1 > > >("loop_LDA");

  // apply BC
  apply_bc->execute();
  // compute element residual distribution
  elem_loop->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
