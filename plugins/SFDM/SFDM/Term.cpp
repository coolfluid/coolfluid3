// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Signal.hpp"
#include "common/OptionComponent.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldGroup.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "solver/CSolver.hpp"
#include "solver/Actions/CLoop.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Term.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::Actions;

namespace cf3 {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

Term::Term ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::solution(), &m_solution))
      ->pretty_name("Solution Field");

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::wave_speed(), &m_wave_speed))
      ->pretty_name("Wave Speed Field");

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::residual(), &m_residual))
      ->pretty_name("Residual Field");

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::jacob_det(), &m_jacob_det))
      ->pretty_name("Jacobian Determinant Field");

}

/////////////////////////////////////////////////////////////////////////////////////

Term::~Term() {}

/////////////////////////////////////////////////////////////////////////////////////

void Term::link_fields()
{
  if( is_null( m_solution.lock() ) )
  {
    m_solution = solver().field_manager()
                         .get_child( SFDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::solution(), m_solution.lock()->uri() );
  }

  if( is_null( m_residual.lock() ) )
  {
    m_residual = solver().field_manager()
                         .get_child( SFDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::residual(), m_residual.lock()->uri() );
  }

  if( is_null( m_wave_speed.lock() ) )
  {
    m_wave_speed = solver().field_manager()
                           .get_child( SFDM::Tags::wave_speed() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::wave_speed(), m_wave_speed.lock()->uri() );
  }

  if( is_null( m_jacob_det.lock() ) )
  {
    m_jacob_det = solver().field_manager()
                          .get_child( SFDM::Tags::jacob_det() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::jacob_det(), m_jacob_det.lock()->uri() );
  }

  if( is_null( m_field_group.lock() ) )
  {
    m_field_group = solution().field_group().as_ptr<FieldGroup>();
  }

}

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
