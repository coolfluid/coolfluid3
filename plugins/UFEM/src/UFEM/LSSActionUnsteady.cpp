// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"

#include "LSSActionUnsteady.hpp"


namespace cf3 {
namespace UFEM {

using namespace common;

common::ComponentBuilder < LSSActionUnsteady, common::ActionDirector, LibUFEM > LSSActionUnsteady_Builder;

LSSActionUnsteady::LSSActionUnsteady(const std::string& name) :
  LSSAction(name)
{
  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&LSSActionUnsteady::trigger_time, this))
    .link_to(&m_time);
}

LSSActionUnsteady::~LSSActionUnsteady()
{
}

Real& LSSActionUnsteady::dt()
{
  return m_dt;
}

Real& LSSActionUnsteady::invdt()
{
  return m_invdt;
}

const solver::Time& LSSActionUnsteady::time() const
{
  return *m_time;
}

void LSSActionUnsteady::trigger_time()
{
  if(is_null(m_time))
      return;

  m_time->options().option("time_step").attach_trigger(boost::bind(&LSSActionUnsteady::trigger_timestep, this));
}

void LSSActionUnsteady::trigger_timestep()
{
  m_invdt = m_time->invdt();
  m_dt = m_time->dt();
}


} // UFEM
} // cf3
