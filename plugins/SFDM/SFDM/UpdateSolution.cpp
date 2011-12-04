// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"

#include "solver/CSolver.hpp"

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
  solver::Action(name)
{
  mark_basic();

  // options

  options().add_option(SFDM::Tags::solution(), m_solution)
     .description("Solution to update")
     .pretty_name("Solution")
     .link_to(&m_solution);

  options().add_option(SFDM::Tags::update_coeff(), m_update_coeff)
     .description("Update coefficient")
     .pretty_name("Update Coefficient")
     .link_to(&m_update_coeff);

  options().add_option(SFDM::Tags::residual(), m_residual)
     .description("Residual")
     .pretty_name("Residual")
     .link_to(&m_residual);
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  link_fields();

  Field& solution     = *m_solution;
  Field& residual     = *m_residual;
  Field& update_coeff = *m_update_coeff;

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
  if( is_null( m_solution ) )
  {
    m_solution = Handle<Field>( follow_link( solver().field_manager()
        .get_child( SFDM::Tags::solution() ) ) );
    options().configure_option( SFDM::Tags::solution(), m_solution->uri() );
  }

  if( is_null( m_update_coeff ) )
  {
    m_update_coeff = Handle<Field>( follow_link( solver().field_manager()
        .get_child( SFDM::Tags::update_coeff() ) ) );
    options().configure_option( SFDM::Tags::update_coeff(), m_update_coeff->uri() );
  }

  if( is_null( m_residual ) )
  {
    m_residual = Handle<Field>( follow_link( solver().field_manager()
        .get_child( SFDM::Tags::residual() ) ) );
    options().configure_option( SFDM::Tags::residual(), m_residual->uri() );
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////////

