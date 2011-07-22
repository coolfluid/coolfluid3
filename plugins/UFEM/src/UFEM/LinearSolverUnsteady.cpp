// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"

#include "Mesh/CNodes.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Tags.hpp"

#include "LinearSolverUnsteady.hpp"


namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;



struct LinearSolverUnsteady::Implementation
{
  void trigger_time()
  {
    if(m_time.expired())
      return;
    
    m_time.lock()->option("time_step").attach_trigger(boost::bind(&Implementation::trigger_timestep, this));
  }
  
  void trigger_timestep()
  {
    m_invdt = m_time.lock()->invdt();
  }
  
  boost::weak_ptr<CTime> m_time;
  Real m_invdt;
};

LinearSolverUnsteady::LinearSolverUnsteady(const std::string& name) :
  LinearSolver(name),
  m_implementation( new Implementation() )
{
  m_options.add_option( OptionComponent<CTime>::create(Solver::Tags::time(), &m_implementation->m_time))
    ->pretty_name("Time")
    ->description("Component that keeps track of time for this simulation")
    ->attach_trigger(boost::bind(&Implementation::trigger_time, m_implementation.get()));
}

LinearSolverUnsteady::~LinearSolverUnsteady()
{
}

Real& LinearSolverUnsteady::invdt()
{
  return m_implementation->m_invdt;
}



} // UFEM
} // CF
