// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Link.hpp"
#include "common/Signal.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "solver/CSolver.hpp"
#include "solver/actions/CLoop.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Term.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

namespace cf3 {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

Term::Term ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  options().add_option(SFDM::Tags::solution(), m_solution)
      .pretty_name("Solution Field")
      .link_to(&m_solution);

  options().add_option(SFDM::Tags::wave_speed(), m_wave_speed)
      .pretty_name("Wave Speed Field")
      .link_to(&m_wave_speed);

  options().add_option(SFDM::Tags::residual(), m_residual)
      .pretty_name("Residual Field")
      .link_to(&m_residual);

  options().add_option(SFDM::Tags::jacob_det(), m_jacob_det)
      .pretty_name("Jacobian Determinant Field")
      .link_to(&m_jacob_det);

}

/////////////////////////////////////////////////////////////////////////////////////

Term::~Term() {}

/////////////////////////////////////////////////////////////////////////////////////

void Term::link_fields()
{
  if( is_null( m_solution ) )
  {
    m_solution = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::solution() ) ) );
    configure_option_recursively( SFDM::Tags::solution(), m_solution );
  }

  if( is_null( m_residual ) )
  {
    m_residual = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::residual() ) ) );
    configure_option_recursively( SFDM::Tags::residual(), m_residual );
  }

  if( is_null( m_wave_speed ) )
  {
    m_wave_speed = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::wave_speed() ) ) );
    configure_option_recursively( SFDM::Tags::wave_speed(), m_wave_speed );
  }

  if( is_null( m_jacob_det ) )
  {
    m_jacob_det = Handle<Field>( follow_link( solver().field_manager().get_child( SFDM::Tags::jacob_det() ) ) );
    configure_option_recursively( SFDM::Tags::jacob_det(), m_jacob_det );
  }

  if( is_null( m_field_group ) )
  {
    m_field_group = solution().field_group().handle<SpaceFields>();
  }

}

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
