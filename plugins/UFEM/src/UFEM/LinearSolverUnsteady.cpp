// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Tags.hpp"

#include "LinearSolverUnsteady.hpp"


namespace cf3 {
namespace UFEM {

using namespace common;

LinearSolverUnsteady::LinearSolverUnsteady(const std::string& name) :
  LinearSolver(name)
{
  options().add_option(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&LinearSolverUnsteady::trigger_time, this))
    .link_to(&m_time);
}

LinearSolverUnsteady::~LinearSolverUnsteady()
{
}

Real& LinearSolverUnsteady::invdt()
{
  return m_invdt;
}

const solver::Time& LinearSolverUnsteady::time() const
{
  return *m_time;
}

void LinearSolverUnsteady::trigger_time()
{
  if(is_null(m_time))
      return;
    
  m_time->options().option("time_step").attach_trigger(boost::bind(&LinearSolverUnsteady::trigger_timestep, this));
}

void LinearSolverUnsteady::trigger_timestep()
{
  m_invdt = m_time->invdt();
}


} // UFEM
} // cf3
