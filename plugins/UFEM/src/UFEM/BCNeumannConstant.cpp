// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "BCNeumannConstant.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < BCNeumannConstant, common::Action, LibUFEM > BCNeumannConstant_Builder;

BCNeumannConstant::BCNeumannConstant(const std::string& name) :
  ProtoAction(name),
  m_rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  regist_signal( "set_tags" )
    .connect( boost::bind( &BCNeumannConstant::signal_set_tags, this, _1 ) )
    .description("Set the tag")
   .pretty_name("Set Tags")
   .signature( boost::bind(&BCNeumannConstant::signature_set_tags, this, _1) );

  options().add("m_flux", 1.)
    .description("variable for flux")
    .pretty_name("flux variable")
    .link_to(&m_flux);

}

BCNeumannConstant::~BCNeumannConstant()
{
}



void BCNeumannConstant::set_tags (const std::string& neumann_variable, const std::string& gradient_field )
{
  FieldVariable<0, ScalarField> neumann(neumann_variable, gradient_field);
  
  set_expression(elements_expression
  (
    boost::mpl::vector1 <mesh::LagrangeP1::Line2D>(), // Valid for surface element types
    m_rhs(neumann) += integral<1>(transpose(N(neumann))*boost::proto::lit(m_flux)*_norm(normal)) // Classical Neumann condition formulation for finite elements
  ));
}

void BCNeumannConstant::signal_set_tags ( common::SignalArgs& node )
{
  common::XML::SignalOptions options(node);
  set_tags(options.option("neumann_field").value<std::string>(),
           options.option("neumann_variable").value<std::string>()
           );
}

void BCNeumannConstant::signature_set_tags ( common::SignalArgs& node )
{
  common::XML::SignalOptions options(node);
  
  options.add("neumann_variable", "Temperature")
    .pretty_name("Neumann BC variable")
    .description("Constant Neumann Boundary Conditions");

  options.add("neumann_field", "")
    .pretty_name("Neumann Field")
    .description("Tag for the field of the Neumann Boundary Condition");
    
}

} // namespace UFEM

} // namespace cf3
