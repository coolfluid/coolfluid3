// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Foreach.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/Field.hpp"
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

  m_options.add_option(OptionComponent<Field>::create("solution", &m_solution))
      ->description("Solution to update")
      ->pretty_name("Solution");

  m_options.add_option(OptionComponent<Field>::create("solution_backup", &m_solution_backup))
      ->description("Solution Backup")
      ->pretty_name("Solution Backup");

  m_options.add_option(OptionComponent<Field>::create("update_coeff", &m_update_coeff))
      ->description("Update coefficient")
      ->pretty_name("Update Coefficient");

  m_options.add_option(OptionComponent<Field>::create("residual", &m_residual))
      ->description("Residual")
      ->pretty_name("Residual");

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

  Field& U  = *m_solution.lock();
  Field& U0 = *m_solution_backup.lock();
  Field& R  = *m_residual.lock();
  Field& H  = *m_update_coeff.lock();

  const Real one_minus_alpha = 1.-m_alpha;
  boost_foreach(const CEntities& elements, U.entities_range())
  {
    CSpace& solution_space = U.space(elements);
    CSpace& P0_space = H.space(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      Real h = H[P0_space.indexes_for_element(e)[0]][0];
      boost_foreach(const Uint state, solution_space.indexes_for_element(e))
      {
        for (Uint j=0; j<U.row_size(); ++j)
        {
          U[state][j] = one_minus_alpha*U0[state][j] + m_alpha*U[state][j] + m_beta*h*R[state][j];
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

