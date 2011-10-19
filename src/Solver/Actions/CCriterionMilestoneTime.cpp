// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Tags.hpp"
#include "Solver/Actions/CCriterionMilestoneTime.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

using namespace common;

ComponentBuilder< CCriterionMilestoneTime, CCriterion, LibActions > CCriterionMilestoneTime_Builder;

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneTime::CCriterionMilestoneTime( const std::string& name  ) :
  CCriterion ( name ),
  m_tolerance(1e-12)
{
  m_properties["brief"] = std::string("Time Criterion object");
  std::string description = properties().value<std::string>("description")+
    "Returns true if a time is reached\n";
  m_properties["description"] = description;

  m_options.add_option(OptionComponent<CTime>::create(Tags::time(), &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time")
      ->mark_basic()
      ->add_tag("time");

  m_options.add_option(OptionT<Real>::create("milestone_dt", 0.))
      ->description("Defines the checkpoints for the criterion")
      ->pretty_name("Milestone Time Step")
      ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneTime::~CCriterionMilestoneTime()
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCriterionMilestoneTime::operator()()
{
  const Real dt = option("milestone_dt").value<Real>();
  if ( dt == 0. )
    return true;

  const Real t = m_time.lock()->current_time();
  return ( t - Uint(t/dt) * dt == 0 );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
