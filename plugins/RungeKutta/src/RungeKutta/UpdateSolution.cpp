// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "RungeKutta/UpdateSolution.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RungeKutta {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < UpdateSolution, CAction, LibRungeKutta > UpdateSolution_Builder;

///////////////////////////////////////////////////////////////////////////////////////

UpdateSolution::UpdateSolution ( const std::string& name ) :
  CAction(name),
  m_alpha(1.),  // forward euler step with these coefficients for alpha and beta
  m_beta(1.)
{
  mark_basic();

  // options

  m_options.add_option(OptionComponent<CField>::create("solution", &m_solution))
      ->description("Solution to update")
      ->pretty_name("Solution");

  m_options.add_option(OptionComponent<CField>::create("solution_backup", &m_solution_backup))
      ->description("Solution Backup")
      ->pretty_name("Solution Backup");

  m_options.add_option(OptionComponent<CField>::create("update_coeff", &m_update_coeff))
      ->description("Update coefficient")
      ->pretty_name("Update Coefficient");

  m_options.add_option(OptionComponent<CField>::create("residual", &m_residual))
      ->description("Residual")
      ->pretty_name("Residual");

  m_solution_view = create_static_component_ptr<CMultiStateFieldView>("solution_view");
  m_solution_backup_view = create_static_component_ptr<CMultiStateFieldView>("solution_backup_view");
  m_residual_view = create_static_component_ptr<CMultiStateFieldView>("residual_view");
  m_update_coeff_view = create_static_component_ptr<CScalarFieldView>("update_coeff_view");

  m_options.add_option(OptionT<Real>::create("alpha", m_alpha))
      ->description("RK coefficient alpha")
      ->pretty_name("alpha")
      ->link_to(&m_alpha);

  m_options.add_option(OptionT<Real>::create("beta", m_beta))
      ->description("RK coefficient beta")
      ->pretty_name("beta")
      ->link_to(&m_beta);
}

////////////////////////////////////////////////////////////////////////////////

void UpdateSolution::execute()
{
  if (m_solution.expired())     throw SetupError(FromHere(), "Solution field was not set");
  if (m_solution_backup.expired())     throw SetupError(FromHere(), "Solution backup field was not set");
  if (m_residual.expired())     throw SetupError(FromHere(), "Residual field was not set");
  if (m_update_coeff.expired()) throw SetupError(FromHere(), "UpdateCoeff Field was not set");

  CScalarFieldView& update_coeff = *m_update_coeff_view;
  CMultiStateFieldView& residual = *m_residual_view;
  CMultiStateFieldView& solution = *m_solution_view;
  CMultiStateFieldView& solution_backup = *m_solution_backup_view;

  /// @todo put this in attached triggers of the field configuration
  residual.set_field(m_residual.lock());
  solution.set_field(m_solution.lock());
  solution_backup.set_field(m_solution_backup.lock());
  update_coeff.set_field(m_update_coeff.lock());

  const Real one_minus_alpha = 1.-m_alpha;
  boost_foreach(const CEntities& elements, m_solution.lock()->field_elements())
  {
    residual.set_elements(elements);
    solution.set_elements(elements);
    solution_backup.set_elements(elements);
    update_coeff.set_elements(elements);

    for (Uint e=0; e<elements.size(); ++e)
    {
      CMultiStateFieldView::View U  = solution[e];
      CMultiStateFieldView::View U0 = solution_backup[e];
      CMultiStateFieldView::View R  = residual[e];
      const Real H                  = update_coeff[e];
      for (Uint i=0; i<U.shape()[0]; ++i)
      {
        for (Uint j=0; j<U.shape()[1]; ++j)
        {
          U[i][j] = one_minus_alpha*U0[i][j] + m_alpha*U[i][j] + m_beta*H*R[i][j];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

