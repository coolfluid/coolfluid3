// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/Signal.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/FieldGroup.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "Solver/Actions/CLoop.hpp"

#include "SFDM/SFDSolver.hpp"
#include "SFDM/CellTerm.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

CellTerm::CellTerm ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::solution(), &m_solution))
      ->pretty_name("Solution Field");

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::wave_speed(), &m_wave_speed))
      ->pretty_name("Wave Speed Field");

  m_options.add_option(OptionComponent<Field>::create( SFDM::Tags::residual(), &m_residual))
      ->pretty_name("Residual Field");
}

/////////////////////////////////////////////////////////////////////////////////////

CellTerm::~CellTerm() {}

/////////////////////////////////////////////////////////////////////////////////////

void CellTerm::link_fields()
{
  if( is_null( m_solution.lock() ) )
  {
    m_solution = solver().as_type<SFDM::SFDSolver>().fields()
                         .get_child( SFDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::solution(), m_solution.lock()->uri() );
  }

  if( is_null( m_residual.lock() ) )
  {
    m_residual = solver().as_type<SFDM::SFDSolver>().fields()
                         .get_child( SFDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::residual(), m_residual.lock()->uri() );
  }

  if( is_null( m_wave_speed.lock() ) )
  {
    m_wave_speed = solver().as_type<SFDM::SFDSolver>().fields()
                         .get_child( SFDM::Tags::wave_speed() ).follow()->as_ptr_checked<Field>();
    configure_option_recursively( SFDM::Tags::wave_speed(), m_wave_speed.lock()->uri() );
  }

  if( is_null( m_field_group.lock() ) )
  {
    m_field_group = solution().field_group().as_ptr<FieldGroup>();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
