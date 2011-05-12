// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "LinearSystemUnsteady.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Solver::Actions::Proto;

LinearSystemUnsteady::LinearSystemUnsteady(const std::string& name) : LinearSystem(name)
{
  Common::Option::Ptr tstep_prop = properties().add_option< OptionT<Real> >("Timestep", "Timestep", 1.);
  tstep_prop->attach_trigger( boost::bind(&LinearSystemUnsteady::trigger_timestep, this) );
  tstep_prop->mark_basic();

  Common::Option::Ptr current_prop = properties().add_option< OptionT<Real> >("CurrentTime", "Current time", 0.);
  current_prop->mark_basic();

  Common::Option::Ptr stop_prop = properties().add_option< OptionT<Real> >("StopTime", "Stoppint time", 1.);
  stop_prop->mark_basic();
}

void LinearSystemUnsteady::on_solve()
{
  const Real start_time = property("CurrentTime").value<Real>();
  const Real stop_time = property("StopTime").value<Real>();
  const Real dt = property("Timestep").value<Real>();

  Real current_time = start_time;
  while(current_time < stop_time)
  {
    CF::UFEM::LinearSystem::on_solve();
    current_time += dt;
  }

  properties()["CurrentTime"].change_value(current_time);
}

void LinearSystemUnsteady::trigger_timestep()
{
  const Real dt = property("Timestep").value<Real>();
  m_invdt = 1. / dt;
}

} // UFEM
} // CF
