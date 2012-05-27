// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"
#include "solver/actions/CriterionMilestoneIteration.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

ComponentBuilder< CriterionMilestoneIteration, Criterion, LibActions > CriterionMilestoneIteration_Builder;

////////////////////////////////////////////////////////////////////////////////

CriterionMilestoneIteration::CriterionMilestoneIteration( const std::string& name  ) :
  Criterion ( name )
{
  properties()["brief"] = std::string("Time Criterion object");
  std::string description = properties().value<std::string>("description")+
    "Returns true if a time is reached\n";
  properties()["description"] = description;

  options().add(Tags::time(), m_time)
      .description("Time tracking component")
      .pretty_name("Time")
      .mark_basic()
      .link_to(&m_time)
      .add_tag("time");

  options().add("milestone_rate", 1u)
      .description("Defines the checkpoints for the criterion")
      .pretty_name("Milestone Rate")
      .mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CriterionMilestoneIteration::~CriterionMilestoneIteration()
{
}

////////////////////////////////////////////////////////////////////////////////

bool CriterionMilestoneIteration::operator()()
{
  /// @bug normally  option("milestone_rate").value<Uint>() should be used
  /// but that throws a bad any_cast exception somehow !?
  const Uint rate = options().value<int>("milestone_rate");
  if ( rate == 0 )
    return false;

  return ( m_time->iter() % rate == 0 );
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
