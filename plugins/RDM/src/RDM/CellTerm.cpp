// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Link.hpp"
#include "common/Signal.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Field.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/CellLoop.hpp"
#include "RDM/CellTerm.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

CellTerm::CellTerm ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  options().add_option(RDM::Tags::solution(), m_solution)
      .pretty_name("Solution Field")
      .link_to(&m_solution);

  options().add_option(RDM::Tags::wave_speed(), m_wave_speed)
      .pretty_name("Wave Speed Field")
      .link_to(&m_wave_speed);

  options().add_option(RDM::Tags::residual(), m_residual)
      .pretty_name("Residual Field")
      .link_to(&m_residual);
}

CellTerm::~CellTerm() {}


void CellTerm::link_fields()
{
  if( is_null( m_solution ) )
  {
    m_solution = follow_link( solver().handle<RDM::RDSolver>()->fields()
                         .get_child( RDM::Tags::solution() ))->handle<Field>();
    configure_option_recursively( RDM::Tags::solution(), m_solution );
  }

  if( is_null( m_residual ) )
  {
    m_residual = follow_link( solver().handle<RDM::RDSolver>()->fields()
                         .get_child( RDM::Tags::residual() ))->handle<Field>();
    configure_option_recursively( RDM::Tags::residual(), m_residual );
  }

  if( is_null( m_wave_speed ) )
  {
    m_wave_speed = follow_link( solver().handle<RDM::RDSolver>()->fields()
                         .get_child( RDM::Tags::wave_speed() ))->handle<Field>();
    configure_option_recursively( RDM::Tags::wave_speed(), m_wave_speed );
  }
}

ElementLoop& CellTerm::access_element_loop( const std::string& type_name )
{
  // ensure that the fields are present

  link_fields();

  // get the element loop or create it if does not exist

  Handle< ElementLoop > loop(get_child( "LOOP" ));
  if( is_null( loop ) )
  {
    const std::string update_vars_type =
        physical_model().get_child( RDM::Tags::update_vars() )
                        ->handle<physics::Variables>()
                        ->type();

    loop = create_component<CellLoop>("LOOP", "CellLoopT<" + type_name + "," + update_vars_type + ">");
  }

  return *loop;
}


/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
