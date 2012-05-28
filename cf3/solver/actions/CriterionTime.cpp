// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "solver/Time.hpp"
#include "solver/Tags.hpp"
#include "solver/actions/CriterionTime.hpp"

using namespace cf3::common;

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CriterionTime, Criterion, LibActions > CriterionTime_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CriterionTime::CriterionTime( const std::string& name  ) :
  Criterion ( name ),
  m_tolerance(1e-12)
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
}


CriterionTime::~CriterionTime() {}


bool CriterionTime::operator()()
{
  if (is_null(m_time)) throw SetupError(FromHere(),"Time option was not set in ["+uri().path()+"]");
  Time& t = *m_time;

  const Real end_time = t.options().value<Real>("end_time");

  bool achieved = ( t.current_time() + m_tolerance > end_time );

  if( achieved )
    CFinfo << "Time limit [" << end_time << "] reached" << CFendl;

  return achieved;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
