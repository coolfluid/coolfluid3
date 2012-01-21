// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "LinearSolverUnsteady.hpp"


namespace cf3 {
namespace UFEM {

using namespace common;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;



struct LinearSolverUnsteady::Implementation
{
  void trigger_time()
  {
    if(is_null(m_time))
      return;
    
    m_time->options().option("time_step").attach_trigger(boost::bind(&Implementation::trigger_timestep, this));
  }
  
  void trigger_timestep()
  {
    m_invdt = m_time->invdt();
  }
  
  Handle<Time> m_time;
  Real m_invdt;
};

LinearSolverUnsteady::LinearSolverUnsteady(const std::string& name) :
  LinearSolver(name),
  m_implementation( new Implementation() )
{
  options().add_option(solver::Tags::time(), m_implementation->m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .attach_trigger(boost::bind(&Implementation::trigger_time, m_implementation.get()))
    .link_to(&m_implementation->m_time);
}

LinearSolverUnsteady::~LinearSolverUnsteady()
{
}

Real& LinearSolverUnsteady::invdt()
{
  return m_implementation->m_invdt;
}



} // UFEM
} // cf3
