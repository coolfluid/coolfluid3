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

#include "solver/Time.hpp"
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

  options().add("my_maxiter", m_max_pseudo_iteration)
      .description("Maximum number of iterations (0 will perform none)")
      .pretty_name("Maximum number")
      .link_to(&m_max_pseudo_iteration).mark_basic();

  options().add("disabled_actions", m_list_of_disabled_actions)
      .description("Actions to be disabled")
      .pretty_name("Disabled actions");

  options().add("time", m_time)
      .description("Current time step")
      .pretty_name("Current time step");

}

CouplingIterationControl::~CouplingIterationControl() {}

void CouplingIterationControl::execute()
{
  CFinfo << "I AM IN EXECUTE()!" << "\n";
  CFinfo << "------------------------------------------------------------" << "\n";
  CFinfo << "m_time is:" << "\n";
  CFinfo << "------------------------------------------------------------" << "\n";
  CFinfo << m_time << "\n";
  if (m_time <= 0.02)
    // m_time != (m_time/10) * 10
    //m_time <= 0.02
    {
      CFinfo << "I AM IN EXECUTE() IF" << "\n";
      CFinfo << "------------------------------------------------------------" << "\n";
      CFinfo << options().option("disabled_actions").value_str() << CFendl;
      m_pseudo_iter_comp->options().set("disabled_actions" , options().option("disabled_actions").value());

    }
  else
    {
      //disabled_actions.clear();
    }
}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
