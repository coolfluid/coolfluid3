// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionComponent.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"

#include "Solver/CSolver.hpp"

#include "SFDM/UpdateSolution.hpp"
#include "SFDM/Tags.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < UpdateSolution, Action, LibSFDM > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

UpdateSolution::UpdateSolution ( const std::string& name ) :
  Solver::Action(name)
{
  mark_basic();

  // options

  m_options.add_option(OptionComponent<Field>::create(SFDM::Tags::solution(), &m_solution))
     ->description("Solution to update")
     ->pretty_name("Solution");

  m_options.add_option(OptionComponent<Field>::create(SFDM::Tags::update_coeff(), &m_update_coeff))
     ->description("Update coefficient")
     ->pretty_name("Update Coefficient");

  m_options.add_option(OptionComponent<Field>::create(SFDM::Tags::residual(), &m_residual))
     ->description("Residual")
     ->pretty_name("Residual");
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  link_fields();

  Field& solution     = *m_solution.lock();
  Field& residual     = *m_residual.lock();
  Field& update_coeff = *m_update_coeff.lock();

  for (Uint i=0; i<solution.size(); ++i)
  {
    for (Uint j=0; j<solution.row_size(); ++j)
    {
      solution[i][j] += update_coeff[i][0] * residual[i][j];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::link_fields()
{
  if( is_null( m_solution.lock() ) )
  {
    m_solution = solver().field_manager()
        .get_child( SFDM::Tags::solution() ).follow()->as_ptr_checked<Field>();
    configure_option( SFDM::Tags::solution(), m_solution.lock()->uri() );
  }

  if( is_null( m_update_coeff.lock() ) )
  {
    m_update_coeff = solver().field_manager()
        .get_child( SFDM::Tags::update_coeff() ).follow()->as_ptr_checked<Field>();
    configure_option( SFDM::Tags::update_coeff(), m_update_coeff.lock()->uri() );
  }

  if( is_null( m_residual.lock() ) )
  {
    m_residual = solver().field_manager()
        .get_child( SFDM::Tags::residual() ).follow()->as_ptr_checked<Field>();
    configure_option( SFDM::Tags::residual(), m_residual.lock()->uri() );
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////////

