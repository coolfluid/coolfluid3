// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PE/Comm.hpp"
#include "common/Action.hpp"
#include "common/DynTable.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/Consts.hpp"
#include "math/Functions.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/Time.hpp"
#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "sdm/Tags.hpp"
#include "sdm/ComputeCellJacobianPerturb.hpp"

#include "sdm/DomainDiscretization.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::math::Consts;
using namespace cf3::math::Functions;

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeCellJacobianPerturb, common::Component, LibSDM > ComputeCellJacobianPerturb_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeCellJacobianPerturb::ComputeCellJacobianPerturb ( const std::string& name ) :
  common::Component(name)
{
  // Define some options
  options().add("domain_discretization", m_domain_discretization )
      .link_to(&m_domain_discretization);

  options().add("residual", m_residual )
      .description("Residual field")
      .pretty_name("Residual")
      .link_to(&m_residual);

  options().add("solution", m_solution)
      .description("Solution field")
      .pretty_name("Solution")
      .link_to(&m_solution);

  options().add("reference_solution", std::vector<Real>())
      .description("Reference solution, required for computation dQ in denominator of dR/dQ")
      .link_to(&m_ref_sol)
      .mark_basic();

  // Compute small number
  m_eps = sqrt(math::Consts::eps());
}

////////////////////////////////////////////////////////////////////////////////

bool ComputeCellJacobianPerturb::loop_cells(const Handle<Cells const>& cells)
{
  // Check configuration
  if ( is_null(m_domain_discretization) ) throw SetupError( FromHere(), "Option 'domain_discretization' not configured for "+uri().string());
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  if ( is_null(m_residual) )              throw SetupError( FromHere(), "Option 'residual' not configured for "+uri().string());
  if ( m_ref_sol.size()==0 )              throw SetupError( FromHere(), "Option 'reference_solution' not configured for "+uri().string());

  // Check if we have to loop over these cells
  if (m_domain_discretization->loop_cells(cells) == false)
    return false; // No domain discretization is defined for these cells --> abort

  // Set the space for these cells and solution
  m_space = m_solution->space(cells);
  cf3_assert(m_space);

  // Set information for thise cells
  m_nb_vars = m_solution->row_size();
  m_nb_sol_pts = m_space->shape_function().nb_nodes();

  // Resize the solution and residual matrices for these cells
  m_Q0.resize(m_nb_sol_pts, m_nb_vars);
  m_R0.resize(m_nb_sol_pts, m_nb_vars);

  return true; // we may proceed to loop over these cells
}

////////////////////////////////////////////////////////////////////////////////

void ComputeCellJacobianPerturb::compute_jacobian(const Uint elem, RealMatrix& cell_jacob)
{
  cf3_assert(m_space);
  cf3_assert(cell_jacob.rows() == m_nb_sol_pts*m_nb_vars);
  cf3_assert(cell_jacob.cols() == m_nb_sol_pts*m_nb_vars);

  // Some allocation
  Real dR, dQ;
  Uint p, dp, s, ds, v, dv;

  Field& Q  = *m_solution;
  Field& R  = *m_residual;

  /// WARNING! The unperturbed residual R must already have been computed beforehand!

  // Backup the unperturbed solution and residual of this element
  for (s=0; s<m_nb_sol_pts; ++s)
  {
    p = m_space->connectivity()[elem][s];

    for (v=0; v<m_nb_vars; ++v)
    {
      m_Q0(s,v) = Q[p][v];
      m_R0(s,v) = R[p][v];
    }
  }

  // contribution of other variables (ds,dv) to this variable (s,v)
  for (ds=0; ds<m_nb_sol_pts; ++ds)
  {
    dp = m_space->connectivity()[elem][ds];

    for (dv=0; dv<m_nb_vars; ++dv)
    {
      // Perturb the other variable (ds,dv)
      dQ = m_eps* sign(Q[dp][dv]) * std::max( std::abs(Q[dp][dv]) , std::abs(m_ref_sol[dv]) );
      cf3_assert(dQ > eps());
      Q[dp][dv] += dQ;

      // Compute the perturbed residual only for this element
      m_domain_discretization->compute_element(elem);

      // Compute the cell jacobian for this element
      for (s=0; s<m_nb_sol_pts; ++s)
      {
        p = m_space->connectivity()[elem][s];
        for (v=0; v<m_nb_vars; ++v)
        {
          dR = R[p][v] - m_R0(s,v);
          cell_jacob(s*m_nb_vars+v,ds*m_nb_vars+dv) = dR/dQ;
        }
      }

      // restore the other variable (ds,dv)
      Q[dp][dv] -= dQ;
    }
  }

  // Restore solution and residual from backup
  for (s=0; s<m_nb_sol_pts; ++s)
  {
    p = m_space->connectivity()[elem][s];

    for (v=0; v<m_nb_vars; ++v)
    {
      Q[p][v] = m_Q0(s,v);
      R[p][v] = m_R0(s,v);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////////

