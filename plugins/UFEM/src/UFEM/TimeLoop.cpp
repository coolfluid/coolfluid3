// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Geometry.hpp"

#include "solver/CTime.hpp"
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
    m_component.options().add_option( OptionComponent<CTime>::create(solver::Tags::time(), &m_time))
    ->pretty_name("Time")
    ->description("Component that keeps track of time for this simulation");
  }
  
  Component& m_component;
  boost::weak_ptr<CTime> m_time;
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
  if(m_implementation->m_time.expired())
    throw common::SetupError(FromHere(), "Error executing TimeLoop " + uri().string() + ": Time is invalid");

  solver::CTime& time = *m_implementation->m_time.lock();
  const Real& t = time.current_time();
  const Real dt = time.dt();
  Uint iter = time.iter();
  while(t < time.end_time())
  {
    ActionDirector::execute();
    time.configure_option("iteration", ++iter);
    time.configure_option("time", dt * static_cast<Real>(iter));
  }
}



} // UFEM
} // cf3
