// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"

#include "solver/CTime.hpp"
#include "solver/Tags.hpp"
#include "solver/actions/CCriterionTime.hpp"

using namespace cf3::common;

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CCriterionTime, CCriterion, LibActions > CCriterionTime_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CCriterionTime::CCriterionTime( const std::string& name  ) :
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
}


CCriterionTime::~CCriterionTime() {}


bool CCriterionTime::operator()()
{
  if (m_time.expired()) throw SetupError(FromHere(),"Time option was not set in ["+uri().path()+"]");
  CTime& t = *m_time.lock();

  const Real end_time = t.option("end_time").value<Real>();

  bool achieved = ( t.current_time() + m_tolerance > end_time );

  if( achieved )
    CFinfo << "Time limit [" << end_time << "] reached" << CFendl;

  return achieved;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
