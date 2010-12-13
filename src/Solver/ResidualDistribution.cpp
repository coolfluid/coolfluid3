// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

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
  
  // signals
  this->regist_signal ( "create_bc" , "Create an apply bc action", "Create BC" )->connect ( boost::bind ( &ResidualDistribution::create_bc, this, _1) );
  signal("create_bc").signature
    .insert<std::string>("Name", "Name of the BC" );
  
  m_elem_loop = create_static_component< CForAllElementsT< CSchemeLDAT< SF::Quad2DLagrangeP1 > > >("loop_LDA");
  
}

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution()
{
}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::create_bc( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");

  
  CLoop::Ptr apply_bc = create_component< CForAllNodes >(name);
    apply_bc->create_action("CF.Actions.CSetFieldValues");
  apply_bc->action("CF.Actions.CSetFieldValues").configure_property("Field",std::string("solution"));
  apply_bc->add_tag("apply_bc_action");
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::compute_rhs()
{
  // apply BC
  boost_foreach (CLoop& apply_bc, find_components_with_tag<CLoop>(*this,"apply_bc_action"))
    apply_bc.execute();
  // compute element residual distribution
  m_elem_loop->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
