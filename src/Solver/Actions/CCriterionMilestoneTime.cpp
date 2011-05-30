// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Actions/CCriterionMilestoneTime.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;

ComponentBuilder< CCriterionMilestoneTime, CCriterion, LibActions > CCriterionMilestoneTime_Builder;

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneTime::CCriterionMilestoneTime( const std::string& name  ) :
  CCriterion ( name ),
  m_tolerance(1e-12)
{
  properties()["brief"] = std::string("Time Criterion object");
  std::string description = properties()["description"].value<std::string>()+
    "Returns true if a time is reached\n";
  properties()["description"] = description;

  properties().add_option(OptionComponent<CTime>::create("time","Time","Time tracking component",&m_time))
    ->mark_basic()
    ->add_tag("time");

  properties().add_option(OptionT<Real>::create("milestone_dt","Milestone Time Step","Defines the checkpoints for the criterion",0.))
      ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneTime::~CCriterionMilestoneTime()
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCriterionMilestoneTime::operator()()
{
  const Real dt = property("milestone_dt").value<Real>();
  if ( dt == 0. )
    return true;

  const Real t = m_time.lock()->time();
  return ( t - Uint(t/dt) * dt == 0 );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
