// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "TimeLoop.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;

struct TimeLoop::Implementation
{
  Implementation(Component& comp) :
   m_component(comp)
  {
    m_component.options().add_option(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .link_to(&m_time);
  }
  
  Component& m_component;
  Handle<Time> m_time;
};

TimeLoop::TimeLoop(const std::string& name) :
  ActionDirector(name),
  m_implementation(new Implementation(*this))
{
}

TimeLoop::~TimeLoop()
{

}

void TimeLoop::execute()
{
  if(is_null(m_implementation->m_time))
    throw common::SetupError(FromHere(), "Error executing TimeLoop " + uri().string() + ": Time is invalid");

  solver::Time& time = *m_implementation->m_time;
  const Real& t = time.current_time();
  const Real dt = time.dt();
  Uint iter = time.iter();
  while(t < time.end_time())
  {
    ActionDirector::execute();
    time.options().configure_option("iteration", ++iter);
    time.options().configure_option("current_time", dt * static_cast<Real>(iter));
  }
}



} // UFEM
} // cf3
