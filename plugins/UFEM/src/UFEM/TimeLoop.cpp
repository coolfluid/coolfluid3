// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/OptionComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/Geometry.hpp"

#include "Solver/CTime.hpp"
#include "Solver/Tags.hpp"

#include "TimeLoop.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace Solver;

struct TimeLoop::Implementation
{
  Implementation(Component& comp) :
   m_component(comp)
  {
    m_component.options().add_option( OptionComponent<CTime>::create(Solver::Tags::time(), &m_time))
    ->pretty_name("Time")
    ->description("Component that keeps track of time for this simulation");
  }
  
  Component& m_component;
  boost::weak_ptr<CTime> m_time;
};

TimeLoop::TimeLoop(const std::string& name) :
  CActionDirector(name),
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

  Solver::CTime& time = *m_implementation->m_time.lock();
  const Real& t = time.current_time();
  const Real dt = time.dt();
  Uint iter = time.iter();
  while(t < time.end_time())
  {
    CActionDirector::execute();
    time.configure_option("iteration", ++iter);
    time.configure_option("time", dt * static_cast<Real>(iter));
  }
}



} // UFEM
} // cf3
