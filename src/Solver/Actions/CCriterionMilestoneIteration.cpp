// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Actions/CCriterionMilestoneIteration.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;

ComponentBuilder< CCriterionMilestoneIteration, CCriterion, LibActions > CCriterionMilestoneIteration_Builder;

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneIteration::CCriterionMilestoneIteration( const std::string& name  ) :
  CCriterion ( name )
{
  m_properties["brief"] = std::string("Time Criterion object");
  std::string description = properties()["description"].value<std::string>()+
    "Returns true if a time is reached\n";
  m_properties["description"] = description;

  m_options.add_option(OptionComponent<CTime>::create("time","Time","Time tracking component",&m_time))
    ->mark_basic()
    ->add_tag("time");

  Uint def = 1;
  m_options.add_option(OptionT<Uint>::create("milestone_rate","Milestone Rate","Defines the checkpoints for the criterion",def))
      ->mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CCriterionMilestoneIteration::~CCriterionMilestoneIteration()
{
}

////////////////////////////////////////////////////////////////////////////////

bool CCriterionMilestoneIteration::operator()()
{
  /// @bug normally  option("milestone_rate").value<Uint>() should be used
  /// but that throws a bad any_cast exception somehow !?
  const Uint rate = option("milestone_rate").value<int>();
  if ( rate == 0 )
    return false;

  return ( m_time.lock()->iter() % rate == 0 );
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
