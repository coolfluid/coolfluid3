// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "CouplingIterationControl.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;

using namespace solver::actions::Proto;
using namespace boost::proto;

////////////////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< CouplingIterationControl, Action, LibUFEM > CouplingIterationControl_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CouplingIterationControl::CouplingIterationControl( const std::string& name  ) :
  Action ( name )
{
  // properties

  // options

  options().add("iterator", m_pseudo_iter_comp)
      .description("Component performing iterations")
      .pretty_name("Iterative component")
      .link_to(&m_pseudo_iter_comp).mark_basic();

  options().add("disabled_actions", m_list_of_disabled_actions)
      .description("Actions to be disabled")
      .pretty_name("Disabled actions");

  options().add("interval", 10)
      .description("Amount of time steps to be executed")
      .pretty_name("Interval")
      .link_to(&m_interval).mark_basic();

  options().add("time", m_time).description("Time component for the simulation").pretty_name("Time").link_to(&m_time);

}

CouplingIterationControl::~CouplingIterationControl() {}

void CouplingIterationControl::execute()
{
  if(is_null(m_time))
    throw SetupError(FromHere(), "Time component not set for CouplingIterationControl at" + uri().path());

  if(is_null(m_pseudo_iter_comp))
    throw SetupError(FromHere(), "Iteration component not set for CouplingIterationControl at" + uri().path());

  const Real time = m_time->current_time();
  const Real dt = m_time->dt();
  const int tstep = static_cast<int>(time/dt);
  CFdebug << "time:" << time << "\n";
  CFdebug << "dt:" << dt << "\n";
  CFdebug << "tstep:" << tstep << "\n";
  CFdebug << "(m_interval):" << (m_interval) << "\n";
  CFdebug << "(tstep % m_interval):" << (tstep % m_interval) << "\n";
  if (tstep % m_interval != 0)
  {
    CFinfo << options().option("disabled_actions").value_str() << CFendl;
    m_pseudo_iter_comp->options().set("disabled_actions" , options().option("disabled_actions").value());
  }
  else
  {
    m_pseudo_iter_comp->options().set("disabled_actions" , std::vector<std::string>());
  }
}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
