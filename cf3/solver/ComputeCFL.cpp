// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"

#include "mesh/Cells.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/Dictionary.hpp"

#include "solver/ComputeCFL.hpp"
#include "solver/Term.hpp"
#include "solver/TermComputer.hpp"
#include "solver/PDE.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;

namespace cf3 {
namespace solver {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeCFL, common::Action, solver::LibSolver > ComputeCFL_Builder;

////////////////////////////////////////////////////////////////////////////////

ComputeCFL::ComputeCFL ( const std::string& name ) : common::Action(name)
{
  options().add("time_step", m_time_step)
    .description("Time step")
    .pretty_name("Time step")
    .mark_basic()
    .link_to(&m_time_step);

  options().add("wave_speed", m_wave_speed)
    .description("Wave Speed divided by characteristic length")
    .pretty_name("Wave Speed")
    .mark_basic()
    .link_to(&m_wave_speed);

  options().add("cfl", m_cfl)
    .description("Courant number, local to a cell")
    .pretty_name("CFL")
    .mark_basic()
    .link_to(&m_cfl);

}

////////////////////////////////////////////////////////////////////////////////

void ComputeCFL::execute()
{
  if ( is_null(m_time_step) )  SetupError(FromHere(), "time_step not configured");
  if ( is_null(m_wave_speed) ) SetupError(FromHere(), "wave_speed not configured");
  
  if ( is_null(m_cfl) )
  {
    Handle<Component> found;
    if ( found = find_component_ptr_with_name( m_wave_speed->dict(), "cfl" ) )
    {
      m_cfl = found->handle<Field>();
    }
    else
    {
      m_cfl = m_wave_speed->dict().create_field("cfl").handle<Field>();
    }
  }
  
  Field& ws  = *m_wave_speed;
  Field& dt  = *m_time_step;
  Field& cfl = *m_cfl;
  // Calculate the CFL number = dt * wave_speed
  for (Uint i=0; i<ws.size(); ++i)
  {
    cfl[i][0] = dt[i][0] * ws[i][0];
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
