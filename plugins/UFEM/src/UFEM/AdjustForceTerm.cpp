// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"

#include "solver/actions/Proto/Expression.hpp"


#include "AdjustForceTerm.hpp"

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < AdjustForceTerm, common::Action, LibUFEM > AdjustForceTerm_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

AdjustForceTerm::AdjustForceTerm ( const std::string& name ) :
  solver::actions::Proto::ProtoAction( name ),
  m_target_velocity(0.),
  m_current_velocity(0.),
  m_dt(1.),
  m_relaxation(1.),
  m_max_force(1.),
  m_direction(0)
{
  options().add("bulk_velocity_computer", m_bulk_velocity_computer)
    .pretty_name("Bulk velocity computer")
    .description("Component that computes the bulk velocity")
    .attach_trigger(boost::bind(&AdjustForceTerm::trigger_bulk_velocity_component, this))
    .mark_basic();
    
  options().add("target_velocity", m_target_velocity)
    .pretty_name("Target Velocity")
    .description("Target value for the bulk velocity")
    .mark_basic()
    .link_to(&m_target_velocity);

  options().add("direction", m_direction)
    .pretty_name("Direction")
    .description("Component of the force field that needs to be adjusted")
    .mark_basic()
    .link_to(&m_direction);

  options().add("relaxation", m_relaxation)
    .pretty_name("Relaxation")
    .description("Relaxation factor for the adjustment")
    .mark_basic()
    .link_to(&m_relaxation);

  options().add("maximum_force", m_max_force)
    .pretty_name("Maximum force")
    .description("Maximum value for the forcing term")
    .mark_basic()
    .link_to(&m_max_force);

  options().add("time", m_time)
    .pretty_name("Time")
    .description("Time component, used to get the time step")
    .mark_basic()
    .link_to(&m_time);
    
  FieldVariable<0, VectorField> body_force("Force", "body_force");
  
  set_expression(nodes_expression(
    body_force[lit(m_direction)] = _min(body_force[lit(m_direction)] + lit(m_relaxation) * (lit(m_target_velocity) - lit(m_current_velocity)) / lit(m_dt), lit(m_max_force))
  ));
}

AdjustForceTerm::~AdjustForceTerm()
{
  if(is_not_null(m_bulk_velocity_computer))
    m_bulk_velocity_computer->options().option("result").detach_trigger(m_bulk_velocity_trigger_id);
}

void AdjustForceTerm::execute()
{
  if(is_null(m_time))
    throw common::SetupError(FromHere(), "Option time is not set for " + uri().path());

  m_dt = m_time->dt();

  solver::actions::Proto::ProtoAction::execute();
}

void AdjustForceTerm::trigger_bulk_velocity_component()
{
  if(is_not_null(m_bulk_velocity_computer))
    m_bulk_velocity_computer->options().option("result").detach_trigger(m_bulk_velocity_trigger_id);
  
  m_bulk_velocity_computer = options().value< Handle<common::Component> >("bulk_velocity_computer");
  
  if(is_not_null(m_bulk_velocity_computer))
  {
    m_bulk_velocity_trigger_id = m_bulk_velocity_computer->options().option("result").attach_trigger_tracked(boost::bind(&AdjustForceTerm::trigger_bulk_velocity, this));
    trigger_bulk_velocity();
  }
}

void AdjustForceTerm::trigger_bulk_velocity()
{
  m_current_velocity = m_bulk_velocity_computer->options().value<Real>("result");
}



} // UFEM
} // cf3
