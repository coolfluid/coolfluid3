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
#include "common/EventHandler.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "KineticEnergyIntegral.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < KineticEnergyIntegral, common::Action, LibUFEM > KineticEnergyIntegral_Builder;

KineticEnergyIntegral::KineticEnergyIntegral(const std::string& name) :
  ProtoAction(name)
{
  options().add("variable_name", "Velocity")
    .pretty_name("Variable Name")
    .description("Name of the variable to use for the velocity")
    .attach_trigger(boost::bind(&KineticEnergyIntegral::trigger_set_expression, this))
    .link_to(&m_variable_name)
    .mark_basic();
 
  options().add("field_tag", "navier_stokes_solution")
    .pretty_name("Field Tag")
    .description("Tag for the field to use")
    .attach_trigger(boost::bind(&KineticEnergyIntegral::trigger_set_expression, this))
    .mark_basic();
    
  options().add("history",m_history)
      .pretty_name("History")
      .description("History component used to log the history of the integral value.")
      .link_to(&m_history)
      .mark_basic();

  trigger_set_expression();
}

KineticEnergyIntegral::~KineticEnergyIntegral()
{
}

void KineticEnergyIntegral::trigger_set_expression()
{
  using boost::proto::lit;
  
  const std::string var = m_variable_name;
  const std::string tag = options().option("field_tag").value<std::string>();
  
  FieldVariable<0, VectorField> u(var, tag);
  
  set_expression(elements_expression(boost::mpl::vector1<mesh::LagrangeP1::Quad2D>(), element_quadrature(lit(m_integral_value) += 0.5*u*transpose(u))));
}

void KineticEnergyIntegral::execute()
{
  if(common::PE::Comm::instance().size() > 1)
  {
    throw common::NotImplemented(FromHere(), "KineticEnergyIntegral is not implemented for parallel runs");
  }

  m_integral_value.resize(1,1);
  m_integral_value.setZero();
  
  solver::actions::Proto::ProtoAction::execute();
  
  if(is_not_null(m_history))
  {
    m_history->set("KineticEnergy", m_integral_value(0,0));
    m_history->save_entry();
  }
  else
  {
    CFwarn << "KineticEnergyIntegral component " << uri().path() << " has no history set." << CFendl;
  }
}


} // namespace UFEM

} // namespace cf3
