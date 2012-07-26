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

namespace cf3 {
namespace sdm {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeCellJacobianPerturb, common::Component, LibSDM > ComputeCellJacobianPerturb_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ComputeCellJacobianPerturb::ComputeCellJacobianPerturb ( const std::string& name ) :
  common::Component(name)
{
  options().add("residual", m_residual )
    .description("Residual field")
    .pretty_name("Residual")
    .link_to(&m_residual);

  options().add("solution", m_solution)
    .description("Solution field")
    .pretty_name("Solution")
    .link_to(&m_solution);

  options().add("reference_solution", std::vector<Real>())
    .description("Reference solution, required for computation of denominator")
    .link_to(&m_ref_sol);

  options().add("domain_discretization", m_domain_discretization )
    .link_to(&m_domain_discretization);

  m_eps = sqrt(math::Consts::eps());
}

////////////////////////////////////////////////////////////////////////////////

bool ComputeCellJacobianPerturb::loop_cells(const Handle<Cells const>& cells)
{

//  CFdebug << "ComputeCellJacobianPerturb set loop" << CFendl;

  if ( is_null(m_domain_discretization) ) throw SetupError( FromHere(), "Option 'domain_discretization' not configured for "+uri().string());
  if ( is_null(m_solution) )              throw SetupError( FromHere(), "Option 'solution' not configured for "+uri().string());
  if ( is_null(m_residual) )              throw SetupError( FromHere(), "Option 'residual' not configured for "+uri().string());

  if (m_domain_discretization->loop_cells(cells) == false)
  {
//    CFinfo << " --> false " << CFendl;
    return false;
  }

  m_space = m_solution->space(cells);
  cf3_assert(m_space);

  m_nb_vars = m_solution->row_size();
  m_nb_sol_pts = m_space->shape_function().nb_nodes();

  m_Q0.resize(m_nb_sol_pts, m_nb_vars);
  m_R0.resize(m_nb_sol_pts, m_nb_vars);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void ComputeCellJacobianPerturb::compute_jacobian(const Uint elem, RealMatrix& cell_jacob)
{
  cf3_assert(m_space);
  cf3_assert(cell_jacob.rows() == m_nb_sol_pts*m_nb_vars);
  cf3_assert(cell_jacob.cols() == m_nb_sol_pts*m_nb_vars);

  Field& Q  = *m_solution;
  Field& R  = *m_residual;


  // Backup and perturb the solution of this element
  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    Uint p = m_space->connectivity()[elem][s];

    for (Uint v=0; v<m_nb_vars; ++v)
    {
      // backup
      cf3_assert(p<Q.size());
      cf3_assert(v<Q.row_size());
      cf3_assert(p<R.size());
      cf3_assert(v<R.row_size());

      m_Q0(s,v) = Q[p][v];
      m_R0(s,v) = R[p][v];
    }
  }


  Real dQ;
  Real dR;
  // contribution of other variables (ds,dv) to this variable (s,v)
  for (Uint ds=0; ds<m_nb_sol_pts; ++ds)
  {
    Uint dp = m_space->connectivity()[elem][ds];

    for (Uint dv=0; dv<m_nb_vars; ++dv)
    {
      if (m_ref_sol.size())
        dQ = m_eps*math::Functions::sign(Q[dp][dv]) * std::max( std::abs(Q[dp][dv]) , std::abs(m_ref_sol[dv]) );
      else
        dQ = m_eps*Q[dp][dv];

      // Perturb the other variable (ds,dv)
      cf3_assert(dp<Q.size());
      cf3_assert(dv<Q.row_size());

      Q[dp][dv] += dQ;

      // Compute the perturbed residual only for this element
      for (Uint s=0; s<m_nb_sol_pts; ++s)
      {
        Uint p = m_space->connectivity()[elem][s];
        for (Uint v=0; v<m_nb_vars; ++v)
        {
          cf3_assert(p<R.size());
          cf3_assert(v<R.row_size());

          R[p][v]  = 0.;  // reset residual of this element
        }
      }
      m_domain_discretization->compute_element(elem);

      // Compute the cell jacobian for this element
      for (Uint s=0; s<m_nb_sol_pts; ++s)
      {
        Uint p = m_space->connectivity()[elem][s];
        for (Uint v=0; v<m_nb_vars; ++v)
        {
          Real dR = R[p][v] - m_R0(s,v);
          cell_jacob(s*m_nb_vars+v,m_nb_vars*ds+dv) = dR/dQ;
        }
      }

      // restore the other variable (ds,dv)
      Q[dp][dv] -= dQ;
    }
  }

  // Restore solution and residual from backup
  for (Uint s=0; s<m_nb_sol_pts; ++s)
  {
    Uint p = m_space->connectivity()[elem][s];

    for (Uint v=0; v<m_nb_vars; ++v)
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

